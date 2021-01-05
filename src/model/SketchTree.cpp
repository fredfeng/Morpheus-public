/*
 * SketchTree.cpp
 *
 *  Created on: Aug 27, 2016
 */

#include "SketchTree.h"
#include <iostream>
#include <new>
#include <sstream>
#include <queue>
#include <stack>
#include <list>
#include "assert.h"
#include"z3++.h"
#include "Util.h"

using namespace std;

SketchTree::SketchTree() {
	cacher_ = false;
	root_ = NULL;
	size_ = 0;
}

SketchTree::~SketchTree() {
	deleteNode(root_);
}

inline string int2str(int n) {
	ostringstream sstream;
	sstream << n;

	return sstream.str();
}

void SketchTree::initByArray(vector<int> vec) {
	//FIXME: Check if it's a complete tree!
	if (vec.size() == 0) {
		return;
	}
	//init root node.
	Node *currNode = createNode(vec[0]);
	root_ = currNode;
	//walk two steps at each loop.
	unsigned doublePtr = 0;
	vector<Node*> nodeVec;
	nodeVec.push_back(currNode);

	//build tree.
	for (unsigned i = 0; i < vec.size(); i++) {
		currNode = nodeVec[i];
		//left child
		doublePtr++;
		if (doublePtr >= vec.size()) {
			break;
		}

		Node *leftChild = createNode(vec[doublePtr]);
		nodeVec.push_back(leftChild);
		currNode->left = leftChild;
		leftChild->parent = currNode;

		//right child
		doublePtr++;
		Node *rightChild = createNode(vec[doublePtr]);
		nodeVec.push_back(rightChild);
		currNode->right = rightChild;
		rightChild->parent = currNode;
	}

}

Node* SketchTree::createNode(int val) {
	Node *node = new (nothrow) Node;
	node->parent = NULL;
	node->left = NULL;
	node->right = NULL;
	node->dataframe = NULL;
	node->cst = NULL;
	node->val = val;
	node->id = size_;
	node->code = "";
	id2Node_[size_] = node;
	size_++;

	node->group_num = 0;

	return node;
}

void SketchTree::cleanTree() {

	Node * node = root_;
	std::deque<Node*> work;
	if (root_ != NULL)
		work.push_back(root_);

	while (!work.empty()){
		node = work.front();
		node->dataframe = NULL;
		node->code = "";
		node->args.clear();
		node->preds.clear();

		work.pop_front();

		if (node->left != NULL)
			work.push_back(node->left);

		if (node->right != NULL)
			work.push_back(node->right);
	}

}

void SketchTree::deleteNode(Node *node) {
	if (node == NULL)
		return;
	deleteNode(node->left);
	deleteNode(node->right);

	delete node;
}

bool SketchTree::isEmpty() const {
	return (root_ == NULL);
}

//(root-left-right)
void SketchTree::preorderTraversal(Node *tree, string &str) const {
	if (tree == NULL)
		return;
	str = str + int2str(tree->val) + " ";
	preorderTraversal(tree->left, str);
	preorderTraversal(tree->right, str);
}

//(left-root-right). Depth-first
void SketchTree::inorderTraversal(Node *tree, string &str) const {
	if (tree == NULL)
		return;
	inorderTraversal(tree->left, str);
	str = str + int2str(tree->val) + " ";
	inorderTraversal(tree->right, str);
}

//(left-right-root)
void SketchTree::postorderTraversal(Node *tree, string &str) const {
	if (tree == NULL)
		return;
	postorderTraversal(tree->left, str);
	postorderTraversal(tree->right, str);
	str = str + int2str(tree->val) + " ";
}

//Non-recursive DFS
void SketchTree::depthFirstTraversal(Node *tree, string &str) const {
	if (tree == NULL)
		return;
	//bool to indicate whether node has been checked
	stack<pair<bool, Node*> > stack;
	Node *node = tree;
	stack.push(make_pair(false, node));

	while (!stack.empty()) {
		pair<bool, Node*> p = stack.top();
		node = p.second;
		stack.pop();

		if (p.first) {
			str += int2str(node->val) + " ";
		} else {
			if (node->right)
				stack.push(make_pair(false, node->right));
			p.first = true;
			stack.push(p);
			if (node->left)
				stack.push(make_pair(false, node->left));
		}
	}
}

// Extract the front node of the given queue
//  Add its children into the queue
// @precondition: given queue has at least one Node
// @return string: front node val
string extractFrontNodeValAppendChildren(queue<Node *> &q) {
	Node *node = q.front();
	q.pop(); //remove the node from queue

	//Add children into the queue
	if (node->left != NULL)
		q.push(node->left);
	if (node->right != NULL)
		q.push(node->right);

	return int2str(node->val);
}

//Breadth-first traversal
void SketchTree::breadthFirstTraversal(Node *tree, string &str) const {
	if (tree == NULL)
		return;
	queue<Node *> q, nextLvlQ;
	Node *node = tree;
	q.push(node);

	while (!q.empty()) {
		str += extractFrontNodeValAppendChildren(q) + " ";
	}
}

string SketchTree::traversal(int traversalEnum) const {
	string str = "";
	if (isEmpty())
		return str;

	switch (traversalEnum) {
	case TraversalType::PREORDER:
		preorderTraversal(root_, str);
		break;
	case TraversalType::INORDER:
		inorderTraversal(root_, str);
		break;
	case TraversalType::POSTORDER:
		postorderTraversal(root_, str);
		break;
	case TraversalType::DEPTHFIRST:
		depthFirstTraversal(root_, str);
		break;
	default:
		breadthFirstTraversal(root_, str);
		break;
	}

	str.resize(str.size() - 1); //remove trailing space. pop_back() in C++11
	return str;
}

Node* findPredecessor(Node *parent, Node *node, int val) {
	if (node == NULL || (node->val == val && parent == NULL))
		return NULL;
	if (node->val == val && parent != NULL)
		return parent;
	cout << "val: " << node->val << endl;

	Node *leftNode = NULL, *rightNode = NULL;
	leftNode = findPredecessor(node, node->left, val);
	rightNode = findPredecessor(node, node->right, val);

	if (leftNode != NULL)
		return leftNode;
	if (rightNode != NULL)
		return rightNode;
	return NULL;
}

Node* findPredecessorToAdd(Node *parent, Node* node, int val) {
	if (node == NULL)
		return parent;
	Node *predecessor = NULL;
	if (val < node->val) {
		predecessor = findPredecessorToAdd(node, node->left, val);
	} else {
		predecessor = findPredecessorToAdd(node, node->right, val);
	}
	return predecessor;
}

void SketchTree::add(Node* newNode) {
	if (isEmpty()) {
		root_ = newNode;
	} else {
		int val = newNode->val;
		Node *parent = findPredecessorToAdd(NULL, root_, val);

		if (val < parent->val) {
			parent->left = newNode;
		} else {
			parent->right = newNode;
		}
	}
}

void SketchTree::add(int val) {
	Node *newNode = createNode(val);
	add(newNode);
}

unsigned long SketchTree::size() const {
	return size_;
}

Node* SketchTree::getRoot() const {
	return root_;
}

/**
 * attach dataframe to its corresponding node.
 */
void SketchTree::attachDataFrame2Node(int id, Rcpp::DataFrame* df) {
	assert(id2Node_.find(id) != id2Node_.end());
	Node* node = id2Node_[id];
	node->dataframe = df;
	assert(df->nrows() != 0);
	std::string strId = std::to_string(id);
	//FIXME:Compute constraint for current node, cache it later!
	std::vector<Rcpp::DataFrame> inputVec;
	for(unsigned j = 0 ; j < node->inputs.size(); j++) {
		int idx = node->inputs[j];
		Rcpp::DataFrame ds = (*inputs)[idx];
		inputVec.push_back(ds);
	}
	z3::expr* df_cst = Util::getDataFrameExpr(*ctx, *df, strId, inputVec);
	node->cst = df_cst;
}

bool traversalChecker(const Node *aNode, const Node *bNode) {
	if (aNode == NULL && bNode == NULL)
		return true;
	if (aNode == NULL || bNode == NULL || aNode->val != bNode->val)
		return false;
	bool areLftEq = traversalChecker(aNode->left, bNode->left);
	bool areRhtEq = traversalChecker(aNode->right, bNode->right);

	return (areLftEq && areRhtEq);
}

//Compare whether 2 trees are identical: O(n)
// Base checks:
//  True - same tree in memory, both nodes are NULL
//  False - trees are diff size,
//          nodes are not NULL and have diff values
bool SketchTree::operator==(SketchTree &tree) const {
	if (&tree == this)
		return true; //same tree in memory
	if (size() != tree.size())
		return false;

	return traversalChecker(root_, tree.getRoot());
}

void addToList(const Node *node, list<int> &list) {
	if (node == NULL) {
		return;
	} else {
		list.push_back(node->id);
	}
	addToList(node->left, list);
	addToList(node->right, list);
}

list<int> SketchTree::toList() const {
	list<int> list;
	addToList(root_, list);
	return list;
}

Rcpp::DataFrame* SketchTree::getDataFrameById(int id) {
	Node* node = id2Node_[id];
	return node->dataframe;
}

Node* SketchTree::getNodeById(int id) {
	Node* node = id2Node_[id];
	return node;
}

void SketchTree::printCodes() {
	list<Node*> worklist;
	std::cout << "print tree**********************\n";
	worklist.push_back(root_);
	while (!worklist.empty()) {
		Node* worker = worklist.front();
		worklist.pop_front();

		if (worker->code.length() > 1) {
			std::string str = "";
			for (std::vector<int>::reverse_iterator i = worker->inputs.rbegin(); i != worker->inputs.rend();
					++i) {
				int id = *i;
				str += (std::to_string(id) + " ");
			}

			std::cout << "ID: " << worker->id << " code: " << worker->code
					<< "input:" << str << std::endl;
		}
		if (worker->left != NULL)
			worklist.push_back(worker->left);

		if (worker->right != NULL)
			worklist.push_back(worker->right);
	}
}

void SketchTree::delDataFrame(Node* cur) {
	if (!cacher_)
	 delete cur->dataframe;
	delete cur->cst;
	cur->dataframe = NULL;
	cur->cst = NULL;
}

