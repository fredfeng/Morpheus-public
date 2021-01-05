#ifndef SketchEncoding_h
#define SketchEncoding_h

#include <Component.h>
#include <Solver.h>
#include <vector>
#include <tuple>
#include <map>
#include <unordered_map>


using namespace Minisat;

class SketchEncoding {

public:
	// pair of components <int=id, bool=<unary,binary>>
	SketchEncoding(std::unordered_map<int, Component*>& components, int k, bool ngram);

	// finds a sketch and stores it in 'sketch'
	void getSketch(std::vector<int>& sketch);

protected:
	Solver solver_;

	std::vector<int> output_;
	std::vector<std::vector<int> > treevariables_;

	std::map<int, int> variable2id_;
	std::map<int, int> id2binary_; // 0 - unary, 1 - binary, 2 - none
	std::map<std::pair<int, int>, int> component2variable_;

	int length_;
	vec<Lit> clause_;

	bool ngram_;

	int nVars();
	int nClauses();

	bool solve();
	bool blockModel();
	bool blockSolution();
	void saveSolution(std::vector<int>& sketch);
	void printSolution();

	void createVars(std::vector<std::pair<int, int> > &components);
	void createConstraints();
	void createEO(vec<Lit>& clause);

	void addUnitClause(Lit a);
	void addBinaryClause(Lit a, Lit b);
	void addTernaryClause(Lit a, Lit b, Lit c);

	std::unordered_map<int, Component*> components_;
	vec<Lit> components_lits_;

	// Cardinality constraint based on cardinality networks
	void createAMK(vec<Lit>& clause, int rhs);

	void CN_hmerge(vec<Lit> &a_s, vec<Lit> &b_s, vec<Lit> &c_s);
	void CN_hsort(vec<Lit> &a_s, vec<Lit> &c_s);
	void CN_smerge(vec<Lit> &a_s, vec<Lit> &b_s, vec<Lit> &c_s);
	void CN_encode(vec<Lit> &a_s, vec<Lit> &c_s, int rhs);

	int64_t rhs_;
	vec<Lit> cn_lits_; //cardinality_outlits


};

#endif
