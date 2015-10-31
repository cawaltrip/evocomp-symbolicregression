/*
* node.cpp
* UIdaho CS-572: Evolutionary Computation
* Node class - a node of a tree data structure
*
* Copyright (C) 2015 Chris Waltrip <walt2178@vandals.uidaho.edu>
*
* This file is part of EC-SymbolicReg
*
* EC-SymbolicReg is free software: you can redistribute it and/or modify
* it under the terms of the GNU Affero General Public License as published by
* the Free Software Foundation, either version 3 of the License, or
* (at your option) any later version.
*
* EC-SymbolicReg is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU Affero General Public License for more details.
*
* You should have received a copy of the GNU Affero General Public License
* along with EC-SymbolicReg.  If not, see <http://www.gnu.org/licenses/>.
*/
#include "node.h"
#include <iostream> /* Only for errors */
#include <random>
#include "operator_types.h"

void Node::Copy(Node *to_copy) {
	op_ = to_copy->op_;
	var_count_ = to_copy->var_count_;
	const_min_ = to_copy->const_min_;
	const_max_ = to_copy->const_max_;

	if (op_ == kConst) {
		const_val_ = to_copy->const_val_;
	} else if (op_ == kVar) {
		var_index_ = to_copy->var_index_;
	} else {
		/* We must not be a leaf node, so copy children too */
		for (size_t i = 0; i < to_copy->children_.size(); ++i) {
			if (to_copy->children_[i] != nullptr) {
				children_[i] = new Node();
				children_[i]->Copy(to_copy->children_[i]);
				children_[i]->parent_ = this;
			}
		}
	}
}
void Node::Erase() {
	for (auto &child : children_) {
		child->parent_ = nullptr;
		child->Erase();
	}
	delete this;
}
void Node::GenerateTree(size_t cur_depth, size_t max_depth, 
	Node *parent, bool full_tree) {
	
	std::random_device rd;
	std::mt19937 mt(rd());

	OpType lower_bound;
	OpType upper_bound;

	parent_ = parent;

	/* Determine bounds for node type */
	if (full_tree) {
		lower_bound = kAdd;
		upper_bound = kDiv;
	} else {
		lower_bound = kAdd;
		upper_bound = kVar;
	}
	if (cur_depth >= max_depth) {
		lower_bound = kConst;
		upper_bound = kVar;
	}
	std::uniform_int_distribution<int> d{ lower_bound,upper_bound };
	op_ = static_cast<OpType>(d(mt));
	switch (op_) {
	case kAdd:
	case kSub:
	case kMult:
	case kDiv:
		for (size_t i = 0; i < 2; ++i) {
			Node *newnode = new Node;
			newnode->var_count_ = this->var_count_;
			newnode->const_min_ = this->const_min_;
			newnode->const_max_ = this->const_max_;
			newnode->GenerateTree(cur_depth + 1, max_depth, 
				this, full_tree);
			children_.push_back(newnode);
		}
		break;
	case kConst:
		const_val_ = GenerateConstantValue();
		break;
	case kVar:
		var_index_ = GenerateVariableIndex();
		break;
	default:
		/* Shouldn't get here */
		std::cerr << "Bad node type!" << std::endl;
		break;
	}
}
void Node::Print() {
	/* TODO (Chris W): Handle this after verifying tree creation works */
}
void Node::CountNodes(size_t &term, size_t &nonterm) {
	for (auto n : children_) {
		if (n != nullptr) {
			n->CountNodes(term, nonterm);
		}
	}
	switch (op_) {
	case kAdd:
	case kSub:
	case kMult:
	case kDiv:
		++nonterm;
		break;
	case kConst:
	case kVar:
		++term;
		break;
	}
}
double Node::Evaluate(std::vector<double> var_values) {
	double left_child;
	double right_child;
	double fitness;

	switch (op_) {
	case kAdd:
		left_child = children_[0]->Evaluate(var_values);
		right_child = children_[1]->Evaluate(var_values);
		fitness = left_child + right_child;
		break;
	case kSub:
		left_child = children_[0]->Evaluate(var_values);
		right_child = children_[1]->Evaluate(var_values);
		fitness = left_child - right_child;
		break;
	case kMult:
		left_child = children_[0]->Evaluate(var_values);
		right_child = children_[1]->Evaluate(var_values);
		fitness = left_child * right_child;
		break;
	case kDiv:
		left_child = children_[0]->Evaluate(var_values);
		right_child = children_[1]->Evaluate(var_values);
		if (right_child == 0) {
			fitness = 1; /* Safe division, need to check with epsilon though */
		} else {
			fitness = left_child / right_child;
		}
		break;
	case kConst:
		fitness = const_val_;
		break;
	case kVar:
		fitness = var_values[var_index_]; /* No error checking here */
		break;
	default:
		std::cerr << "Invalid node type!" << std::endl;
		fitness = -1; /* Should error out eventually */
		break;
	}
	return fitness;
}
void Node::SetParent(Node *parent) {
	parent_ = parent;
}
void Node::SetVarCount(size_t var_count) {
	var_count_ = var_count;
}
void Node::SetConstMin(double const_min) {
	const_min_ = const_min;
}
void Node::SetConstMax(double const_max) {
	const_max_ = const_max;
}
bool Node::IsTerminal() {
	if (op_ == kConst || op_ == kVar) {
		return true;
	}
	return false;
}
bool Node::IsNonTerminal() {
	return !IsTerminal();
}
double Node::GenerateConstantValue() {
	std::random_device rd;
	std::mt19937 mt(rd());

	std::uniform_real_distribution<double> v{ const_min_,const_max_ };
	return v(mt);
}
size_t Node::GenerateVariableIndex() {
	std::random_device rd;
	std::mt19937 mt(rd());

	std::uniform_int_distribution<size_t> v{ 0,var_count_ };
	return v(mt);
}