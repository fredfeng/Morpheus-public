/*
 * Deduction.h
 *
 *  Created on: Aug 26, 2016
 *      Author: yu
 */

#ifndef DEDUCTION_DEDUCTION_H_
#define DEDUCTION_DEDUCTION_H_

#include <vector>
#include <string>
#include"z3++.h"
#include <unordered_map>
#include"SketchTree.h"
#include"Component.h"
#include"Benchmark.h"

using namespace z3;

class Deduction {
private:

	std::unordered_map<int, Component*> components_;

	std::unordered_map<std::string, expr> cstMap_;

	bool debug_ = false;

	bool hasSpec2_ = false;

	bool hasSpec3_ = false;

	bool unSatCore_ = false;

	int cstCnt_ = 1;

	std::vector<expr*> garbage_;

	SketchTree* sketchTree_;

	z3::solver* solver_;

	context* ctx_;

	//first: id of the tree node; second: index of input table
	std::unordered_map<int, int> node2InputIndex_;

	std::vector<Rcpp::DataFrame> inputs_;

public:
	Deduction();

	Deduction(std::unordered_map<int, Component*> &components);

//	bool unify(std::vector<int>& sketch, expr output, std::vector<expr> inputs);

	expr* attachConstraints(Node* node, context& ctx, int outId, std::string transfer);

	expr getNodeConstraint(Node* node, context& ctx);

	void blockSolution();

	bool solve();

	bool is_sat();

	void buildSystem(std::vector<int>& sketch, Benchmark *benchmark, RInside *R);

	bool alignInputTables();

	void setInputTables(std::vector<Rcpp::DataFrame> &tables);

	void attachInputDataFrames();

	SketchTree* getTree();

	std::unordered_map<int,int>& getNode2Input();

	void addCst(expr e);

	void clean();

	expr getAlignModel();

	void printUnsatCore();

	void setCacheR(bool cacher){
		sketchTree_->setCacheR(cacher);
	}

	virtual ~Deduction();
};

#endif /* DEDUCTION_DEDUCTION_H_ */
