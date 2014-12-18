/*
 * ExpressionParser.cpp
 *
 *  Created on: Dec 18, 2014
 *      Author: robert
 */

#include "ExpressionParser.h"
#include <iostream>
#include <sstream>
#include <list>
#include <stack>
#include <map>
#include <string>
#include <vector>
#include <iterator>
#include <stdlib.h>

using namespace std;

ExpressionParser::ExpressionParser() {

}

ExpressionParser::~ExpressionParser() {

}

// Test if token is an pathensesis
bool ExpressionParser::isParenthesis(const string& token) {
	return token == "(" || token == ")";
}

// Test if token is an operator
bool ExpressionParser::isOperator(const string& token) {
	return token == "+" || token == "-" || token == "*" || token == "/";
}

// Test associativity of operator token
bool ExpressionParser::isAssociative(const string& token, const int& type) {
	const pair<int, int> p = opmap.find(token)->second;
	return p.second == type;
}

// Compare precedence of operators.
int ExpressionParser::cmpPrecedence(const string& token1,
		const string& token2) {
	const pair<int, int> p1 = opmap.find(token1)->second;
	const pair<int, int> p2 = opmap.find(token2)->second;

	return p1.first - p2.first;
}

// Convert infix expression format into reverse Polish notation
bool ExpressionParser::infixToRPN(const vector<string>& inputTokens,
		const int& size, vector<string>& strArray) {
	bool success = true;

	list<string> out;
	stack<string> stack;

	// While there are tokens to be read
	for (int i = 0; i < size; i++) {
		// Read the token
		const string token = inputTokens[i];

		// If token is an operator
		if (isOperator(token)) {
			// While there is an operator token, o2, at the top of the stack AND
			// either o1 is left-associative AND its precedence is equal to that of o2,
			// OR o1 has precedence less than that of o2,
			const string o1 = token;

			if (!stack.empty()) {
				string o2 = stack.top();

				while (isOperator(o2)
						&& ((isAssociative(o1, LEFT_ASSOC)
								&& cmpPrecedence(o1, o2) == 0)
								|| (cmpPrecedence(o1, o2) < 0))) {
					// pop o2 off the stack, onto the output queue;
					stack.pop();
					out.push_back(o2);

					if (!stack.empty())
						o2 = stack.top();
					else
						break;
				}
			}

			// push o1 onto the stack.
			stack.push(o1);
		}
		// If the token is a left parenthesis, then push it onto the stack.
		else if (token == "(") {
			// Push token to top of the stack
			stack.push(token);
		}
		// If token is a right bracket ')'
		else if (token == ")") {
			// Until the token at the top of the stack is a left parenthesis,
			// pop operators off the stack onto the output queue.
			string topToken = stack.top();

			while (topToken != "(") {
				out.push_back(topToken);
				stack.pop();

				if (stack.empty())
					break;
				topToken = stack.top();
			}

			// Pop the left parenthesis from the stack, but not onto the output queue.
			if (!stack.empty())
				stack.pop();

			// If the stack runs out without finding a left parenthesis,
			// then there are mismatched parentheses.
			if (topToken != "(") {
				return false;
			}
		}
		// If the token is a number, then add it to the output queue.
		else {
			out.push_back(token);
		}
	}

	// While there are still operator tokens in the stack:
	while (!stack.empty()) {
		const string stackToken = stack.top();

		// If the operator token on the top of the stack is a parenthesis,
		// then there are mismatched parentheses.
		if (isParenthesis(stackToken)) {
			return false;
		}

		// Pop the operator onto the output queue./
		out.push_back(stackToken);
		stack.pop();
	}

	strArray.assign(out.begin(), out.end());

	return success;
}

double ExpressionParser::RPNtoDouble(vector<string> tokens) {
	stack<string> st;

	// For each token
	for (int i = 0; i < (int) tokens.size(); ++i) {
		const string token = tokens[i];

		// If the token is a value push it onto the stack
		if (!isOperator(token)) {
			st.push(token);
		} else {
			double result = 0.0;

			// Token is an operator: pop top two entries
			const string val2 = st.top();
			st.pop();
			const double d2 = strtod(val2.c_str(), NULL);

			if (!st.empty()) {
				const string val1 = st.top();
				st.pop();
				const double d1 = strtod(val1.c_str(), NULL);

				//Get the result
				result = token == "+" ? d1 + d2 : token == "-" ? d1 - d2 :
							token == "*" ? d1 * d2 : d1 / d2;
			} else {
				if (token == "-")
					result = d2 * -1;
				else
					result = d2;
			}

			// Push result onto stack
			ostringstream s;
			s << result;
			st.push(s.str());
		}
	}

	return strtod(st.top().c_str(), NULL);
}

vector<string> ExpressionParser::getExpressionTokens(const string& expression) {
	vector<string> tokens;
	string str = "";

	for (int i = 0; i < (int) expression.length(); ++i) {
		const string token(1, expression[i]);

		if (isOperator(token) || isParenthesis(token)) {
			if (!str.empty()) {
				tokens.push_back(str);
			}
			str = "";
			tokens.push_back(token);
		} else {
			// Append the numbers
			if (!token.empty() && token != " ") {
				str.append(token);
			} else {
				if (str != "") {
					tokens.push_back(str);
					str = "";
				}
			}
		}
	}

	return tokens;
}

// Print iterators in a generic way
template<typename T, typename InputIterator>
void ExpressionParser::Print(const string& message,
		const InputIterator& itbegin, const InputIterator& itend,
		const string& delimiter) {
	/*cout << message << endl;

	copy(itbegin, itend, ostream_iterator<T>(cout, delimiter.c_str()));

	cout << endl; */
}

int ExpressionParser::parse(string s) {
	// string s = "( 1 + 2) * ( 3 / 4 )-(5+6)";

	Print<char, s_iter>("Input expression:", s.begin(), s.end(), "");

	// Tokenize input expression
	vector<string> tokens = getExpressionTokens(s);

	// Evaluate feasible expressions
	vector<string> rpn;
	if (infixToRPN(tokens, tokens.size(), rpn)) {
		double d = RPNtoDouble(rpn);
		Print<string, cv_iter>("RPN tokens:  ", rpn.begin(), rpn.end(), " ");

		// cout << "Result = " << d << endl;
	} else {
		// cout << "Mis-match in parentheses" << endl;
	}

	return 0;
}
