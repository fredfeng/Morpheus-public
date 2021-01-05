/*
 * Deduction.cpp
 *
 *  Created on: Aug 26, 2016
 *      Author: yu
 */

#include "Deduction.h"
#include "SketchTree.h"
#include "Util.h"
#include"Benchmark.h"

#include <iostream>
#include <string>
#include"z3++.h"
#include <unordered_map>
#include <list>
#include <vector>
#include <exception>
#include <sstream>

using namespace z3;

using namespace std;

Deduction::Deduction() {

}

Deduction::Deduction(std::unordered_map<int, Component*> &components) {
	components_ = components;
}

expr Deduction::getAlignModel() {
	expr_vector curr_models(*ctx_);
	for (unordered_map<int, int>::iterator it = node2InputIndex_.begin();
			it != node2InputIndex_.end(); ++it) {
		int inputId = node2InputIndex_[it->first];
		string blockExpr = "df_" + to_string(it->first) + "_"
				+ to_string(inputId);
		expr block = ctx_->bool_const(blockExpr.c_str());
		curr_models.push_back(block);
	}
	expr model = Util::mk_and(curr_models);
	return model;
}

void Deduction::blockSolution() {
	//block current model.
	expr model = this->getAlignModel();
	solver_->add(!model);
}

bool Deduction::solve() {
	solver_->push();
	//step0: Enforce alignment models!
	addCst(getAlignModel());
//	std::cout << "current model: " << getAlignModel() << std::endl;
	//step1: Traverse each node in the sketchTree and collect all constraints of dataframes, if any.
	list<int> mylist = sketchTree_->toList();
//	cout << "list size: " << mylist.size() << "\n";
	for (list<int>::iterator it = mylist.begin(); it != mylist.end(); ++it) {
		int nodeId = *it;
		Node* node = sketchTree_->getNodeById(nodeId);
		if (node->cst != NULL) {
			addCst(*node->cst);
		}

	}

	check_result res = solver_->check();

	bool flag = (res == sat);
//	if (!flag) {
//		std::cout << "constraint system: " << solver_->to_smt2() << std::endl;
//		this->printUnsatCore();
//	}
	//rollback to original context.
	solver_->pop();
	return flag;
}

bool Deduction::is_sat() {
	check_result res = solver_->check();
	bool flag = (res == sat);
	return flag;
}

void Deduction::buildSystem(std::vector<int>& sketch, Benchmark *benchmark,
		RInside *R) {
	ctx_ = new context();

	//FIXME.
	bool multi = (benchmark->getInput().size() > 1);
	expr output = Util::getDataFrameExpr(*ctx_, *R, benchmark->getOutput(),
			benchmark->getInput(), 1, 0, multi);
	//std::cout << "output_cst: " << output_cst << "\n";
	std::vector<expr> inputs;
	for (unsigned i = 0; i < benchmark->getInput().size(); i++) {
		std::string input_df = benchmark->getInput()[i];
		std::vector<std::string> vs;
		vs.push_back(input_df);
		expr input_cst = Util::getDataFrameExpr(*ctx_, *R, benchmark->getOutput(),
				vs, 0, i, multi);
		inputs.push_back(input_cst);
		//std::cout << "input_cst: " << input_cst << "\n";
	}

	sketchTree_ = new SketchTree();
	stringstream ss;

	solver_ = new solver(*ctx_);
	///unsat core.
	if (this->unSatCore_) {
		params p(*ctx_);
		p.set(":unsat-core", true);
		solver_->set(p);
	}
    //////////////
	vector<Node*> leafs;
	sketchTree_->initByArray(sketch);
	sketchTree_->ctx = ctx_;

	// BFS on current tree.
	if (debug_) {
		cout << "current sketch:"
				<< sketchTree_->traversal(TraversalType::BREADTHFIRST) << "\n";
	}
	std::list<Node*> worklist;
	Node* root = sketchTree_->getRoot();
	worklist.push_back(root);

	// Constraints for each node based on the specs.
	while (!worklist.empty()) {
		Node *worker = worklist.front();
		worklist.pop_front();
		int val = worker->val;
		//save leaf node for solving.
		if (val == 0) {
			leafs.push_back(worker);
			continue;
		}
		// visit each node.
		assert((val != -1) && (val != 0));
		expr nodeCst = getNodeConstraint(worker, *ctx_);
		//	cout << "[node constraints]: " << *nodeCsts << endl;
		addCst(nodeCst);

		//left?
		if (worker->left != NULL) {
			int lftVal = worker->left->val;
			if (lftVal != -1) {
				worklist.push_back(worker->left);
			}
		}
		//right?
		if (worker->right != NULL) {
			int rtVal = worker->right->val;
			if (rtVal != -1) {
				worklist.push_back(worker->right);
			}
		}
	}

	// Root node and output constraints are the same.
	addCst(output);
	if (debug_) {
		cout << "[output constraints]: " << output << endl;
	}

	if (hasSpec2_ || hasSpec3_) {
		expr out_row = ctx_->int_const("ROW_OUT_99_0");
		expr out_col = ctx_->int_const("COL_OUT_99_0");
		expr root_row = ctx_->int_const("ROW_0");
		expr root_col = ctx_->int_const("COL_0");
		//introduce equality.
		expr conj_out1 = (out_row == root_row);
		expr conj_out2 = (out_col == root_col);
		addCst(conj_out1);
		addCst(conj_out2);
	}

	//equality for reasoning the new columns in the output table
	if (hasSpec3_) {
		expr out_head = ctx_->int_const("HEAD_OUT_99_0");
		expr root_head = ctx_->int_const("HEAD_0");
		expr conj_out3 = (out_head == root_head);
		addCst(conj_out3);

		expr out_content = ctx_->int_const("CONTENT_OUT_99_0");
		expr root_content = ctx_->int_const("CONTENT_0");
		expr conj_out4 = (out_content == root_content);
		addCst(conj_out4);

		expr out_group = ctx_->int_const("GROUP_OUT_99_0");
		expr root_group = ctx_->int_const("GROUP_0");
		expr conj_out5 = (out_group == root_group);
		addCst(conj_out5);
	}

	// Each input node should be consistent with at least one input constraint
	for (std::size_t i = 0; i < leafs.size(); i++) {
		Node* leaf = leafs[i];
		string leaf_row = Util::append("ROW_", leaf->id);
		string leaf_col = Util::append("COL_", leaf->id);
		expr leaf_row_cst = ctx_->int_const(leaf_row.c_str());
		expr leaf_col_cst = ctx_->int_const(leaf_col.c_str());
		expr_vector disjunct_vec_rows(*ctx_);
		expr_vector disjunct_vec_cols(*ctx_);
		expr_vector disjunct_vec_head(*ctx_);
		expr_vector disjunct_vec_content(*ctx_);
		expr_vector disjunct_vec_group(*ctx_);
		expr_vector xor_vec_model(*ctx_);

		string leaf_head = Util::append("HEAD_", leaf->id);
		expr leaf_head_cst = ctx_->int_const(leaf_head.c_str());

		string leaf_content = Util::append("CONTENT_", leaf->id);
		expr leaf_content_cst = ctx_->int_const(leaf_content.c_str());

		string leaf_group = Util::append("GROUP_", leaf->id);
		expr leaf_group_cst = ctx_->int_const(leaf_group.c_str());

		for (std::size_t j = 0; j < inputs.size(); j++) {
			expr in = inputs[j];
			addCst(in);
			if (debug_) {
				cout << "[input constraints]: " << in << endl;
			}

			string rowInput = Util::append("ROW_IN_99_", j);
			string colInput = Util::append("COL_IN_99_", j);
			string align_model = Util::append(
					"df_" + std::to_string(leaf->id) + "_", j);

			expr pair = ctx_->bool_const(align_model.c_str());
			expr row1 = ctx_->int_const(rowInput.c_str());
			expr col1 = ctx_->int_const(colInput.c_str());
			//introduce equality.
			expr conj_in1 = (leaf_row_cst == row1);
			expr conj_in2 = (leaf_col_cst == col1);
			disjunct_vec_rows.push_back(conj_in1);
			disjunct_vec_cols.push_back(conj_in2);
			xor_vec_model.push_back(pair);
			addCst(implies(pair, conj_in1));
//			solver_->add(implies(conj_in1, pair));

			string headInput = Util::append("HEAD_IN_99_", j);
			expr head1 = ctx_->int_const(headInput.c_str());
			expr conj_in3 = (head1 == leaf_head_cst);
			disjunct_vec_head.push_back(conj_in3);

			string contentInput = Util::append("CONTENT_IN_99_", j);
			expr content1 = ctx_->int_const(contentInput.c_str());
			expr conj_in4 = (content1 == leaf_content_cst);
			disjunct_vec_content.push_back(conj_in4);

			string groupInput = Util::append("GROUP_IN_99_", j);
			expr group1 = ctx_->int_const(groupInput.c_str());
			expr conj_in5 = (group1 == leaf_group_cst);
			disjunct_vec_group.push_back(conj_in5);

		}
		expr disj1 = Util::mk_or(disjunct_vec_rows);
		expr disj2 = Util::mk_or(disjunct_vec_cols);
		expr disj3 = Util::mk_or(disjunct_vec_head);
		expr disj4 = Util::mk_or(disjunct_vec_content);
		expr disj5 = Util::mk_or(disjunct_vec_group);
		expr disj6 = Util::mk_xor(xor_vec_model);

		if (debug_) {
			cout << "[disjunct constraints1]: " << disj1 << endl;
			cout << "[disjunct constraints2]: " << disj2 << endl;
			cout << "[disjunct constraints6]: " << disj6 << endl;
		}

		if (hasSpec2_ || hasSpec3_) {
			addCst(disj1);
			addCst(disj2);
		}
		if (hasSpec3_) {
			addCst(disj3);
			addCst(disj4);
			addCst(disj5);
		}
		addCst(disj6);
	}

	// All input tables should be used by at least one component.
	for (std::size_t j = 0; j < inputs.size(); j++) {
		expr_vector defused_vec(*ctx_);
		for (std::size_t i = 0; i < leafs.size(); i++) {
			Node* leaf = leafs[i];
			string align_model = Util::append(
					"df_" + std::to_string(leaf->id) + "_", j);
			expr pair = ctx_->bool_const(align_model.c_str());
			defused_vec.push_back(pair);
		}
		expr defuse1 = Util::mk_or(defused_vec);
		if (debug_) {
			cout << "[def-used constraints1]: " << defuse1 << endl;
		}

		addCst(defuse1);
	}
}

SketchTree* Deduction::getTree() {
	return sketchTree_;
}

expr Deduction::getNodeConstraint(Node* node, context& ctx) {
	int id = node->id;
	int val = node->val;
	Component* comp = components_[val];
	vector<string> csts = comp->getConstraint();
	expr_vector conjunct_vec_spec(ctx);

	for (unsigned i = 0; i < csts.size(); i++) {
		string transfer = csts[i];
		//FIXME: This part is ugly. Change it later!
		if (Util::contains(transfer, "RO_SPEC")) {
			if(Util::contains(transfer, "true")) {
				continue;
			}
			hasSpec2_ = true;
			string output_row = Util::append("ROW_", id);
			string output_col = Util::append("COL_", id);
			// Current node is the output.
			Util::replaceAll(transfer, "RO_SPEC", output_row);
			Util::replaceAll(transfer, "CO_SPEC", output_col);

			//FIXME:Temporary hack:
			if(Util::contains(transfer, "OG_SPEC")) {
				string output_group = Util::append("GROUP_", id);
				Util::replaceAll(transfer, "OG_SPEC", output_group);
				Node* left = node->left;
				assert(left != NULL);
				int leftId = left->id;
				string left_group = Util::append("GROUP_", leftId);
				Util::replaceAll(transfer, "IG_SPEC", left_group);
			}

			// left child: can not be null
			Node* left = node->left;
			assert(left != NULL);
			int leftId = left->id;
			string left_row = Util::append("ROW_", leftId);
			string left_col = Util::append("COL_", leftId);
			Util::replaceAll(transfer, "RI_SPEC", left_row);
			Util::replaceAll(transfer, "CI_SPEC", left_col);

			// right child: it may be null
			Node* right = node->right;
			if (right != NULL) {
				int rightId = right->id;
				string rt_row = Util::append("ROW_", rightId);
				string rt_col = Util::append("COL_", rightId);
				Util::replaceAll(transfer, "RI2_SPEC", rt_row);
				Util::replaceAll(transfer, "CI2_SPEC", rt_col);
			}
		} else if (Util::contains(transfer, "SET_O_SPEC")) {
			string output_set = Util::append("SET_", id);
			// Current node is the output.
			Util::replaceAll(transfer, "SET_O_SPEC", output_set);

			// left child: can not be null
			Node* left = node->left;
			assert(left != NULL);
			int leftId = left->id;
			string left_row = Util::append("SET_", leftId);
			Util::replaceAll(transfer, "SET_I_SPEC", left_row);

			// right child: it may be null
			Node* right = node->right;
			if (right != NULL) {
				int rightId = right->id;
				string rt_set = Util::append("SET_", rightId);
				Util::replaceAll(transfer, "SET_I2_SPEC", rt_set);
			}
		} else if (Util::contains(transfer, "OC_SPEC")
				|| Util::contains(transfer, "OH_SPEC")
				|| Util::contains(transfer, "OG_SPEC")) {
			hasSpec3_ = true;
			string output_head = Util::append("HEAD_", id);
			string output_content = Util::append("CONTENT_", id);
			string output_group = Util::append("GROUP_", id);
			// Current node is the output.
			Util::replaceAll(transfer, "OH_SPEC", output_head);
			Util::replaceAll(transfer, "OC_SPEC", output_content);
			Util::replaceAll(transfer, "OG_SPEC", output_group);
			// left child: can not be null
			Node* left = node->left;
			assert(left != NULL);
			int leftId = left->id;
			string left_head = Util::append("HEAD_", leftId);
			string left_content = Util::append("CONTENT_", leftId);
			string left_group = Util::append("GROUP_", leftId);
			Util::replaceAll(transfer, "IH_SPEC", left_head);
			Util::replaceAll(transfer, "IC_SPEC", left_content);
			Util::replaceAll(transfer, "IG_SPEC", left_group);

			// right child: it may be null
			Node* right = node->right;
			if (right != NULL) {
				int rightId = right->id;
				string rt_head = Util::append("HEAD_", rightId);
				string rt_content = Util::append("CONTENT_", rightId);
				string rt_group = Util::append("GROUP_", rightId);
				Util::replaceAll(transfer, "IH2_SPEC", rt_head);
				Util::replaceAll(transfer, "IC2_SPEC", rt_content);
				Util::replaceAll(transfer, "IG2_SPEC", rt_group);
			}
		}

		expr* in = Util::convertStrToExpr(ctx, transfer);
		conjunct_vec_spec.push_back(*in);
		//track garbage.
//		garbage_.push_back(in);
	}

	//conjoin everything together.
	expr conj_expr = Util::mk_and(conjunct_vec_spec);
	return conj_expr;
}

void Deduction::attachInputDataFrames() {

	/// Clear previous inputs of each nodes.
	list<int> mylist = sketchTree_->toList();
	for (list<int>::iterator it = mylist.begin(); it != mylist.end(); ++it) {
		int nodeId = *it;
		Node* node = sketchTree_->getNodeById(nodeId);
		if (node->inputs.size() > 0) {
			node->inputs.clear();
		}
	}

	for (unordered_map<int, int>::iterator it = node2InputIndex_.begin();
			it != node2InputIndex_.end(); ++it) {
		int nodeId = it->first;
		int inputIdx = node2InputIndex_[nodeId];
		unsigned int check = inputIdx;

		////////////////////
		Node* node = sketchTree_->getNodeById(nodeId);
		//start from itself!!!
		Node* parent = node;
        while(parent != NULL) {
        	parent->inputs.push_back(inputIdx);
        	parent = parent->parent;
        }
		assert(check < inputs_.size());
		Rcpp::DataFrame* df = &inputs_[inputIdx];
		sketchTree_->inputs = &inputs_;
//		std::cout << "generate input alignment----------------:" << nodeId << " inputIdx:" << inputIdx << " attachIdx:" << node->inputs[0] << std::endl ;
		this->sketchTree_->attachDataFrame2Node(nodeId, df);
	}
}

bool Deduction::alignInputTables() {
	node2InputIndex_.clear();
	sketchTree_->cleanTree(); // clean previous state
	//I also need to fill in the dataframes of input tables.
	check_result res = solver_->check();
	bool flag = (res == sat);
	if (flag) {
		model m = solver_->get_model();
		// traversing the model
		for (unsigned i = 0; i < m.size(); i++) {
			func_decl v = m[i];
			// this problem contains only constants
			assert(v.arity() == 0);
			expr interp = m.get_const_interp(v);

			if (Util::startsWith("df_", v.name().str())
					&& Util::isTrueExpr(interp)) {
				//interpret the model;
				vector<string> splits = Util::split(v.name().str(), '_');
				assert(splits.size() == 3);
				int nodeId = Util::str2int(splits[1]);
				int inputIdx = Util::str2int(splits[2]);

				node2InputIndex_[nodeId] = inputIdx;
				if (debug_) {
					cout << "save model: " << v.name() << " " << nodeId << " "
							<< inputIdx << endl;
				}
				//attach dataframe to current tree.
				this->attachInputDataFrames();
			}
		}
	}
	return flag;
}

void Deduction::setInputTables(std::vector<Rcpp::DataFrame> &tables) {
	inputs_ = tables;
}

std::unordered_map<int,int>& Deduction::getNode2Input() {
	return node2InputIndex_;
}

void Deduction::addCst(z3::expr e) {
	if (unSatCore_) {
		int val = cstCnt_;
		std::string id = std::to_string(val);
		expr valExpr = ctx_->bool_const(id.c_str());
		std::string key = Util::expr2String(valExpr);
		cstMap_.insert(std::make_pair(key, e));
		solver_->add(e, valExpr);
		cstCnt_++;
	} else {
		solver_->add(e);
	}
}

void Deduction::printUnsatCore() {
	std::cout << "UNSAT CORE====================\n";
	for (unsigned i = 0; i < solver_->unsat_core().size(); i++) {
		expr e = solver_->unsat_core()[i];
		std::string s = Util::expr2String(e);
		assert(cstMap_.count(s));
		expr core = cstMap_.find(s)->second;
		std::cout << "[unsat core]" << core << "\n";
	}
}

void Deduction::clean() {
	solver_->reset();
	delete solver_;
	delete sketchTree_;
	delete ctx_;
	Z3_finalize_memory();
}

Deduction::~Deduction() {
	// FIXME: will lead to segmentation fault when deleting garbaga_ and solver_

	// clean garbages.
	for (unsigned i = 0; i < garbage_.size(); i++) {
		delete garbage_[i];
	}
	//delete solver_;
}

