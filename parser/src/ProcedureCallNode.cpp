/*
 * ProcedureCallNode.cpp
 *
 *  Created on: Jan 8, 2015
 *      Author: robert
 */

#include <iostream>
#include "ProcedureCallNode.h"

using namespace std;

ProcedureCallNode::ProcedureCallNode() {
	parameter_exps = vector<ExpressionNode*>();
}

ProcedureCallNode::~ProcedureCallNode() {

}

string ProcedureCallNode::getProcedureName() {
	return procedure_name;
}

void ProcedureCallNode::setProcedureName(string a_procedure_name) {
	procedure_name = a_procedure_name;
}

void ProcedureCallNode::emit(CodeGenerator* cg) {
	//
	// put the parameters on the stack
	//
	for (vector<ExpressionNode*>::iterator it = parameter_exps.begin();
			it != parameter_exps.end(); ++it) {
		cg->emitRpn((*it)->getRpn());
	}
	//
	// emit a "cal"
	// leave the call address 0, since this will be corrected in the fix stage
	//
	cg->emit(5, 0, 0);
	cg->addCallAddress(cg->getHere() - 2, procedure_name);
}

void ProcedureCallNode::addParametersExpression(ExpressionNode* en) {
	parameter_exps.push_back(en);
}