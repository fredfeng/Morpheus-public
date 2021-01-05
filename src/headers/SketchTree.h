/*
 * SketchTree.h
 *
 *  Created on: Aug 27, 2016
 */

#ifndef MODEL_SKETCHTREE_H_
#define MODEL_SKETCHTREE_H_

#include <string>
#include <vector>
#include <queue>
#include <list>
#include <RInside.h>
#include"z3++.h"

struct Node {
	int id;
	int val;
	Node *parent;
	Node *left;
	Node *right;
	Rcpp::DataFrame* dataframe;
	z3::expr* cst;
	std::string code;
	std::vector<std::string> args;
	std::vector<std::vector<std::string>> preds;

	std::set<std::string> group_char;
	int group_num;

	std::vector<int> inputs;
};

namespace TraversalType {
enum Traversal_E {
	PREORDER, INORDER, POSTORDER, DEPTHFIRST, BREADTHFIRST
};
}

class SketchTree {

	Node *root_;

	int size_;

	bool cacher_;

	std::unordered_map<int, Node*> id2Node_;

	Node* createNode(int val);
	void deleteNode(Node *node);

	// Traversals as per defined in TraversalType enum
	void preorderTraversal(Node *tree, std::string &str) const;
	void inorderTraversal(Node *tree, std::string &str) const;
	void postorderTraversal(Node *tree, std::string &str) const;

	void depthFirstTraversal(Node *tree, std::string &str) const;
	void breadthFirstTraversal(Node *tree, std::string &str) const;
	void populateQueueWithNextLvlNodes(std::queue<Node *> &q,
			std::string &str) const;

public:
	SketchTree();

	z3::context* ctx;

	std::vector<Rcpp::DataFrame>* inputs;

	bool isEmpty() const;

	unsigned long size() const;

	void add(int val);

	void initByArray(std::vector<int> vec);

	void add(Node* newNode);

	Node* getRoot() const;

	void attachDataFrame2Node(int id, Rcpp::DataFrame* df);

	std::string traversal(int traversalEnum = 1) const;

	/* Compare whether 2 trees of  are identical
	 *  O(n) since we have to visit every node */
	bool operator==(SketchTree &tree) const;

	std::list<int> toList() const; //Convert tree values to list

	Rcpp::DataFrame* getDataFrameById(int id);

	Node* getNodeById(int id);

	void delDataFrame(Node* cur);

	void printCodes();

	void cleanTree();

	void setCacheR(bool cacher){
		cacher_ = cacher;
	}

	virtual ~SketchTree();
};

#endif /* MODEL_SKETCHTREE_H_ */
