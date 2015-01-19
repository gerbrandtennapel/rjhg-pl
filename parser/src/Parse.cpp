/*
 * Parse.cpp
 *
 *  Created on: Dec 15, 2014
 *      Author: robert
 */

#include <iostream>
#include <fstream>
#include <vector>
#include "Parse.h"
#include "PException.h"
#include "ExpressionParser.h"
#include "ProcedureNode.h"
#include "ProcedureCallNode.h"
#include "ReturnNode.h"
#include "IfNode.h"

using namespace std;

Parse::Parse() {
	found_char = ' ';
	offset = 0;
	ep = ExpressionParser();
}

Parse::~Parse() {

}

ProgramNode Parse::getPn() {
	return pn;
}

void Parse::start(string filename) {
	ifstream myfile(filename.c_str(), ios::binary);
	streampos begin, end;
	begin = myfile.tellg();
	myfile.seekg(0, ios::end);
	end = myfile.tellg();
	int size = end - begin;
	//
	// reading it into a buffer
	//
	vector<char> a_buffer(size);
	buffer = a_buffer;
	myfile.clear();
	myfile.seekg(0, ios::beg);
	if (!myfile.read(buffer.data(), size)) {
		cout << "File " << filename << " could not be opened" << endl;
	}
	//
	// buffer now contains it
	//
	myfile.close();
	parse_from_memory();
}

void Parse::parse_from_memory() {
	//
	// read the contents from buffer, and make a parse tree
	//
	offset = 0;
	get_something(" \n\t\r");
	for (;;) {
		code_definition();
		if (peek_string.empty()) {
			return;
		}
	}
}

//
// grab something until one of "chars" is reached
//
//
void Parse::get_something(string chars) {
	for (;;) {
		bool result = get_onething(chars);
		if (!result) {
			peek_string = "";
			break;
		}
		if (!peek_string.empty()) {
			break;
		}
	}
}

void Parse::code_definition() {
// cout << "in code_definition peek_string = <" << peek_string << ">" << endl;
	if (peek_string == "class") {
		class_definition();
	} else if (peek_string == "method") {
		method_definition();
	} else if (peek_string == "instance_variable") {
		instance_variable_definition();
	} else if (peek_string == "proc") {
		procedure_definition();
	} else {
		immediate_code();
	}
}

/**
 * trim off whitespace
 */
void Parse::trim(string& s) {
	s.erase(s.find_last_not_of(" \n\r\t") + 1);
	unsigned int first_useful = s.find_first_not_of(" \n\r\t");
	if ((first_useful > 0) && (first_useful != string::npos)) {
		s.erase(0, first_useful);
	}
}

//
// get something until one of chars is reached
//
// return true if there is more to get, false if it's the end
//
// peek_string is the found thing
// found_char is the next character after that
//
bool Parse::get_onething(string chars) {
	if (chars == "") {
		chars = " \n\t\r";
	}
	peek_string = "";
	for (;;) {
		if (offset >= buffer.size()) {
			return false;
		}
		char c = buffer[offset];
		offset++;
		unsigned int found = chars.find(c);
		//
		// -1: not found
		//
		if (found == string::npos) {
			peek_string += c;
		} else {
			break;
		}
	}
//
// strip the peek_string
//
	trim(peek_string);
//
// do some more lookahead, until we find a non blank found_char
//
	lookahead();
	return true;
}

void Parse::lookahead() {
	unsigned int my_offset = offset - 1;
	char c = ' ';
	for (;;) {
		c = buffer[my_offset];
		my_offset++;
		if (c > 32) {
			found_char = c;
			break;
		}
		if (my_offset >= buffer.size()) {
			break;
		}
	}
	// cout << "[" << (unsigned int) c << "=" << c << "]" << endl;
}

void Parse::class_definition() {
	cout << "class_definition" << endl;
	get_something(" \n\t\r");
//
// get the class name
//
	string class_name = peek_string;
	get_something(" \n\t\r");
//
// now expect instance_variables, or method keyword
//
	if (peek_string == "instance_variables") {
		instance_variable_definition();
	} else if (peek_string == "method") {
		method_definition();
	} else {
		cout << "expected instance_variables or method but got <" << peek_string
				<< ">" << endl;
	}

}
void Parse::method_definition() {

	cout << "method_definition" << endl;
	get_something(" \n\t\r");
	string method_name = peek_string;
	get_something(" \n\t\r");

}

void Parse::procedure_definition() {
	ProcedureNode* pd = new ProcedureNode();
	//
	// get the definition
	//
	get_something("(\r\n");
	string proc_name = peek_string;
	//  cout << "PROCEDURE_DEFINITION " << proc_name << endl;
	pd->setName(proc_name);
	for (;;) {
		get_something("),");
		pd->addParameter(peek_string);
		if (found_char == ')') {
			// done
			break;
		}
	}
	vector<Statement*>* statements = block(pd);
	pd->setStatements(statements);
	get_something(" \n\t\r");
	pn.addProcedure(pd);
}

vector<Statement*>* Parse::block(ProcedureNode* pd) {
	vector<Statement*>* statements = new vector<Statement*>;
	for (;;) {
		get_something(" \t(=\n\r");
		//cout << "peek_string = <" << peek_string << ">" << endl;
		//cout << "found_char = <" << found_char << ">" << endl;
		if ((peek_string == "end") || (peek_string == "endif")
				|| (peek_string == "else")) {
			break;
		}
		//
		// return, assignment or proc call or if statement
		//
		if (peek_string == "return") {
			cout << "DECISION: return" << endl;
			statements->push_back(return_statement(pd));
		} else if (peek_string == "if") {
			statements->push_back(if_statement(pd));
		} else if (found_char == '=') {
			//
			// must be an assignment
			//
			statements->push_back(assignment(pd));
		} else {
			//
			// assume procedure call
			//
			cout << "DECISION: proc call" << endl;
			procedure_call(pd);
		}
	}
	return statements;
}

Statement* Parse::assignment(ProcedureNode* pd) {
	string assignment_left = peek_string;
	get_something("=\n\r");
	//
	// look up the instance variable
	//
	unsigned int i = pd->getInstanceVariable(assignment_left);
	string assignment_right = peek_string;
	cout << "ASSIGNMENT_EXPRESSION = <" << assignment_right << ">" << endl;
	ExpressionNode* en = ep.parse(assignment_right);
	//
	// create assignment node with new, to avoid it going out of scope
	//
	AssignmentNode* an = new AssignmentNode(i, en);
	return an;
}

void Parse::instance_variable_definition() {

	cout << "instance_variable_definition" << endl;
	for (;;) {
		get_something(" \n\t\r");
		if (peek_string == "end") {
			get_something(" \n\t\r");
			return;
		}
		cout << "INSTANCE VARIABLE " << peek_string << endl;
	}

}

void Parse::local_variable_definition() {
}

void Parse::immediate_code() {

	throw PException("unexpected string " + peek_string);

}

Statement* Parse::procedure_call(ProcedureNode* pd) {
// cout << "---------------------> procedure_call" << endl;
	string proc_name = peek_string;
// cout << "name " << proc_name << endl;
	ProcedureCallNode* pcn = new ProcedureCallNode();
	pcn->setProcedureName(proc_name);
	for (;;) {
		get_something("),\r\n");
		// cout << "parameter " << peek_string << endl;
		if (found_char == ')') {
			// done
			break;
		} else {
			// parameter
			string parameter_expression = peek_string;
			cout << "PROC Parameter_EXPRESSION = <" << parameter_expression
					<< ">" << endl;
			ExpressionNode* en = ep.parse(parameter_expression);
			pcn->addParametersExpression(en);
		}
	}
	return pcn;
}

Statement* Parse::return_statement(ProcedureNode* pd) {
	get_something("\n\r");
	string return_expression = peek_string;
	cout << "RETURN_EPRESSION = <" << return_expression << ">" << endl;
	ExpressionNode* en;
	if (return_expression.empty()) {
		en = NULL;
	} else {
		en = ep.parse(return_expression);
	}
	ReturnNode* rn = new ReturnNode(en);
	return (Statement*) rn;
}

Statement* Parse::if_statement(ProcedureNode* pd) {
	get_something("\n\r");
	string if_expression = peek_string;
	ExpressionNode* en = ep.parse(if_expression);
	vector<Statement*>* s_true = block(pd);
	vector<Statement*>* s_false;
	if (peek_string == "else") {
		s_false = block(pd);
	} else {
		s_false = NULL;
	}
	IfNode* in = new IfNode(en, s_true, s_false);
	return in;
}
