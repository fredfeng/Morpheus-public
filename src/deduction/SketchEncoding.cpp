#include <SketchEncoding.h>

#include <math.h>
#include <iostream>

SketchEncoding::SketchEncoding(std::unordered_map<int, Component*> &components, int k, bool ngram) {

	components_ = components;
	std::vector<std::pair<int, int> > sketch_components;

	for (const auto &pair : components) {
		int tables = 0;
		for (const auto &type : components[pair.first]->getArgTypes()) {
			if (!type.compare("Table")) {
				tables++;
			}
		}
		assert(tables == 1 || tables == 2);
		sketch_components.push_back(
				std::make_pair(components[pair.first]->getId(), tables - 1));
	}

	length_ = k;
	int size = pow(2, k + 1) - 1;
	for (int i = 0; i < size; i++) {
		output_.push_back(-1);
		std::vector<int> tmp;
		treevariables_.push_back(tmp);
	}

	sketch_components.push_back(std::make_pair(-1, 2));
	sketch_components.push_back(std::make_pair(0, 2));

	ngram_ = ngram;

	if (!ngram_) {
		createVars(sketch_components);
		createConstraints();
	}
}

void SketchEncoding::getSketch(std::vector<int>& sketch) {

	bool res = solve();
	if (res) {
		// SATISFIABLE
		saveSolution(sketch);
		blockSolution();
	} else {
		// if it is UNSATISFIABLE then sketch will be empty
		sketch.clear();
	}

}

int SketchEncoding::nVars() {
	return solver_.nVars();
}

int SketchEncoding::nClauses() {
	return solver_.nClauses();
}

bool SketchEncoding::solve() {
	return solver_.solve();
}

bool SketchEncoding::blockModel() {

	assert(solver_.model.size() > 0);
	clause_.clear();

	for (int i = 0; i < nVars(); i++) {
		if (solver_.modelValue(i) == l_True) {
			clause_.push(~mkLit(i));
		} else {
			clause_.push(mkLit(i));
		}
	}

	bool ok = solver_.addClause(clause_);
	clause_.clear();

	return ok;
}

bool SketchEncoding::blockSolution() {

	assert(output_.size() > 0);
	clause_.clear();

	int size = pow(2, length_ + 1) - 1;

	for (int i = 0; i < size; i++) {
		for (int j = 0; j < (int) treevariables_[i].size(); j++) {
			if (solver_.modelValue(treevariables_[i][j]) == l_True) {
				clause_.push(~mkLit(treevariables_[i][j]));
			}
		}
	}

	bool ok = solver_.addClause(clause_);
	clause_.clear();

	return ok;
}

void SketchEncoding::saveSolution(std::vector<int>& sketch) {

	sketch.clear();
	assert(solver_.model.size() > 0);

	int size = pow(2, length_ + 1) - 1;

	for (int i = 0; i < size; i++) {
		for (int j = 0; j < (int) treevariables_[i].size(); j++) {
			if (solver_.modelValue(treevariables_[i][j]) == l_True) {
				output_[i] = variable2id_[treevariables_[i][j]];
			}
		}
	}

	int amk = 0;
	for (int i = 0; i < components_lits_.size(); i++) {
		if (solver_.modelValue(components_lits_[i]) == l_True)
			amk++;
	}
	assert(amk == length_);

	for (const auto &key : output_) {
		sketch.push_back(key);
	}
	assert(sketch.size() > 0);
}

void SketchEncoding::printSolution() {
	for (int i = 0; i < (int) output_.size(); i++) {
		std::cout << "[" << output_[i] << "] ";
	}
	std::cout << std::endl;
}

// Create the basic variables for the sketch
void SketchEncoding::createVars(std::vector<std::pair<int, int> > &components) {

	int varid = 0;
	int size = pow(2, length_ + 1) - 1;

	for (int j = 0; j < (int) components.size(); j++) {
		id2binary_[components[j].first] = components[j].second;
		for (int i = 0; i < size; i++) {
			solver_.newVar();

			variable2id_[varid] = components[j].first;

			std::pair<int, int> p(components[j].first, i);
			component2variable_[p] = varid;

			treevariables_[i].push_back(varid);
			varid++;
		}
	}

}

// Encoding for the sketch
void SketchEncoding::createConstraints() {

	int size = pow(2, length_ + 1) - 1;

	//1) leafs can only be empty or input tables
	int leafs = pow(2, length_);

	for (int i = leafs - 1; i < size; i++) {
		for (int j = 0; j < (int) treevariables_[i].size(); j++) {
			assert(
					variable2id_.find(treevariables_[i][j])
							!= variable2id_.end());
			if (variable2id_[treevariables_[i][j]] != 0
					&& variable2id_[treevariables_[i][j]] != -1) {
				addUnitClause(~mkLit(treevariables_[i][j]));
			}
		}
	}

	//2)a if component is unary then left child is not empty and right child is empty
	//2)b if component is binary then left and right children are not empty
	//2)c if component is none then left and right children are empty
	for (int i = 0; i < leafs - 1; i++) {
		for (int j = 0; j < (int) treevariables_[i].size(); j++) {
			assert(
					variable2id_.find(treevariables_[i][j])
							!= variable2id_.end());
			assert(
					id2binary_.find(variable2id_[treevariables_[i][j]])
							!= id2binary_.end());

			if (id2binary_[variable2id_[treevariables_[i][j]]] == 0) {
				// unary
				int left = 2 * i + 1;
				int parent = (i-1) / 2;
				Lit a = ~mkLit(treevariables_[i][j]);
				std::pair<int, int> pl(-1, left);
				assert(
						component2variable_.find(pl)
								!= component2variable_.end());
				Lit b = ~mkLit(component2variable_[pl]);
				addBinaryClause(a, b);

				// if the node is the root then do not do group_by
				if (i == 0 && variable2id_[treevariables_[i][j]] == 7) {
					addUnitClause(a);
				}

				if (i > 0) {

					// binary relations
					// if node is group_by => parent node is summarize
					if (variable2id_[treevariables_[i][j]] == 7) {
						std::pair<int, int> summarize(6, parent);
						if (component2variable_.find(summarize)
								!= component2variable_.end()) {
							Lit b = mkLit(component2variable_[summarize]);
							addBinaryClause(a, b);
						}
					}

					if (variable2id_[treevariables_[i][j]] == 3) {
						// if select => ~filter
						std::pair<int, int> filter(10, parent);
						if (component2variable_.find(filter)
								!= component2variable_.end()) {
							Lit b = ~mkLit(component2variable_[filter]);
							addBinaryClause(a, b);
						}

//						// if select => ~select
//						std::pair<int, int> select(3, parent);
//						if (component2variable_.find(select)
//								!= component2variable_.end()) {
//							Lit b = ~mkLit(component2variable_[select]);
//							addBinaryClause(a, b);
//						}
					}

					// if op => parent is not op
					// allow repeated filter and unite to simulate conjunctions
					if (variable2id_[treevariables_[i][j]] != 10
							&& variable2id_[treevariables_[i][j]] != 5) {

						std::pair<int, int> twice(
								variable2id_[treevariables_[i][j]], parent);
						if (component2variable_.find(twice)
								!= component2variable_.end()) {
							Lit b = ~mkLit(component2variable_[twice]);
							addBinaryClause(a, b);
						}
					}
				}

				int right = 2 * i + 2;
				std::pair<int, int> pr(-1, right);
				assert(
						component2variable_.find(pr)
								!= component2variable_.end());
				Lit c = mkLit(component2variable_[pr]);
				addBinaryClause(a, c);

			} else if (id2binary_[variable2id_[treevariables_[i][j]]] == 1) {
				// binary
				int left = 2 * i + 1;
				Lit a = ~mkLit(treevariables_[i][j]);
				std::pair<int, int> pl(-1, left);
				assert(
						component2variable_.find(pl)
								!= component2variable_.end());
				Lit b = ~mkLit(component2variable_[pl]);
				addBinaryClause(a, b);

				int right = 2 * i + 2;
				std::pair<int, int> pr(-1, right);
				assert(
						component2variable_.find(pr)
								!= component2variable_.end());
				Lit c = ~mkLit(component2variable_[pr]);
				addBinaryClause(a, c);

			} else {
				// none
				int left = 2 * i + 1;
				Lit a = ~mkLit(treevariables_[i][j]);
				std::pair<int, int> pl(-1, left);
				assert(
						component2variable_.find(pl)
								!= component2variable_.end());
				Lit b = mkLit(component2variable_[pl]);
				addBinaryClause(a, b);

				int right = 2 * i + 2;
				std::pair<int, int> pr(-1, right);
				assert(
						component2variable_.find(pr)
								!= component2variable_.end());
				Lit c = mkLit(component2variable_[pr]);
				addBinaryClause(a, c);
			}
		}
	}

	//3) Exactly-one component occurs at each node
	for (int i = 0; i < size; i++) {
		clause_.clear();
		for (int j = 0; j < (int) treevariables_[i].size(); j++) {
			clause_.push(mkLit(treevariables_[i][j]));
		}
		createEO(clause_);
		clause_.clear();
	}

	//4) Number of components non empty, non input is exactly length_
	for (int i = 0; i < leafs - 1; i++) {
		Lit p = mkLit(solver_.nVars());
		components_lits_.push(p);
		solver_.newVar();

		for (int j = 0; j < (int) treevariables_[i].size(); j++) {
			Lit q = mkLit(treevariables_[i][j]);

			if (variable2id_[treevariables_[i][j]] != 0
					&& variable2id_[treevariables_[i][j]] != -1) {
				addBinaryClause(~q, p);
			} else {
				addBinaryClause(~q, ~p);
			}
		}
	}

	createAMK(components_lits_, length_);

	clause_.clear();
	for (int i = 0; i < (int) components_lits_.size(); i++) {
		clause_.push(~components_lits_[i]);
	}
	createAMK(clause_, clause_.size() - length_);
	clause_.clear();

	//5) Restrictions on the number of times a component occurs
	std::vector<int> unary;
	std::vector<int> binary;

	for (int j = 0; j < (int) treevariables_[0].size(); j++) {
		if (id2binary_[variable2id_[treevariables_[0][j]]] == 0)
			unary.push_back(j);
		else if (id2binary_[variable2id_[treevariables_[0][j]]] == 1)
			binary.push_back(j);
	}

	// At most each unary component can appear twice -- heuristic
	for (int j = 0; j < (int) unary.size(); j++) {
		clause_.clear();
		for (int i = 0; i < size; i++) {
			Lit q = mkLit(treevariables_[i][unary[j]]);
			clause_.push(q);
		}
		if (clause_.size() > 2) {
			createAMK(clause_, 2);
		}
		clause_.clear();
	}

	// At most one binary component can occur -- heuristic
	clause_.clear();
	for (int i = 0; i < size; i++) {
		for (int j = 0; j < (int) binary.size(); j++) {
			Lit q = mkLit(treevariables_[i][binary[j]]);
			clause_.push(q);
		}
	}
	if (clause_.size() > 1)
		createAMK(clause_, 1);
	clause_.clear();

}

// Encodings for EO and AMK constraints
void SketchEncoding::createEO(vec<Lit>& clause) {

	vec<Lit> lits;
	clause.copyTo(lits);
	assert(lits.size() != 0);

	if (lits.size() == 1) {
		addUnitClause(lits[0]);
	} else {

		vec<Lit> aux;

		for (int i = 0; i < lits.size() - 1; i++) {
			aux.push(mkLit(solver_.nVars(), false));
			solver_.newVar();
		}

		for (int i = 0; i < lits.size(); i++) {
			if (i == 0) {
				addBinaryClause(lits[i], ~aux[i]);
				addBinaryClause(~lits[i], aux[i]);
			} else if (i == lits.size() - 1) {
				addBinaryClause(lits[i], aux[i - 1]);
				addBinaryClause(~lits[i], ~aux[i - 1]);
			} else {
				addBinaryClause(~aux[i - 1], aux[i]);
				addTernaryClause(lits[i], ~aux[i], aux[i - 1]);
				addBinaryClause(~lits[i], aux[i]);
				addBinaryClause(~lits[i], ~aux[i - 1]);
			}
		}
	}
}

void SketchEncoding::createAMK(vec<Lit> &lits, int rhs) {

	assert(rhs >= 0);
	assert(lits.size() > 0);

	rhs_ = rhs;

	cn_lits_.clear();
	vec<Lit> lits_copy;
	lits.copyTo(lits_copy);

	if (rhs == 0) {
		for (int i = 0; i < lits.size(); i++)
			addUnitClause(~lits[i]);
		return;
	}

	vec<Lit> units;
	int n = lits_copy.size();
	// Find the smallest power of 2 that is larger than rhs.
	int new_k = pow(2, floor(log2(rhs)) + 1);
	int m = ceil((double) n / new_k) * new_k - n;

	// The size of the input variables must be a multiple of rhs.
	for (int i = 0; i < m; ++i) {
		Lit p = mkLit(solver_.nVars(), false);
		solver_.newVar();
		lits_copy.push(p);
		units.push(~p);
	}

	for (int i = 0; i < new_k; ++i) {
		cn_lits_.push(mkLit(solver_.nVars(), false));
		solver_.newVar();
	}

	// Enforce that the cardinality constraint can take at most k true elements.
	// We can add as unit clauses all values between k and new_k.
	for (int i = rhs; i < new_k; i++)
		units.push(~cn_lits_[i]);

	CN_encode(lits_copy, cn_lits_, new_k);

	for (int i = 0; i < units.size(); i++)
		addUnitClause(units[i]);
}

void SketchEncoding::CN_hmerge(vec<Lit> &a_s, vec<Lit> &b_s, vec<Lit> &c_s) {

	assert(a_s.size() == b_s.size());
	assert(a_s.size() * 2 == c_s.size());

	if (a_s.size() == 1) {
		Lit c1 = c_s[0];
		Lit c2 = c_s[1];

		Lit a = a_s[0];
		Lit b = b_s[0];

		addTernaryClause(~a, ~b, c2);
		addBinaryClause(~a, c1);
		addBinaryClause(~b, c1);
	} else {

		vec<Lit> odd_a_s;
		vec<Lit> even_a_s;
		vec<Lit> odd_b_s;
		vec<Lit> even_b_s;

		for (int i = 0; i < a_s.size(); i = i + 2)
			odd_a_s.push(a_s[i]);

		for (int i = 1; i < a_s.size(); i = i + 2)
			even_a_s.push(a_s[i]);

		for (int i = 0; i < b_s.size(); i = i + 2)
			odd_b_s.push(b_s[i]);

		for (int i = 1; i < b_s.size(); i = i + 2)
			even_b_s.push(b_s[i]);

		vec<Lit> d_s;
		vec<Lit> e_s;

		d_s.push(c_s[0]);

		for (int i = 1; i < a_s.size(); ++i) {
			d_s.push(mkLit(solver_.nVars(), false));
			solver_.newVar();
		}

		for (int i = 0; i < a_s.size() - 1; ++i) {
			e_s.push(mkLit(solver_.nVars(), false));
			solver_.newVar();
		}

		e_s.push(c_s[c_s.size() - 1]);

		for (int i = 1; i < a_s.size(); ++i) {
			addTernaryClause(~d_s[i], ~e_s[i - 1], c_s[2 * i]);
			addBinaryClause(~d_s[i], c_s[2 * i - 1]);
			addBinaryClause(~e_s[i - 1], c_s[2 * i - 1]);
		}

		CN_hmerge(odd_a_s, odd_b_s, d_s);
		CN_hmerge(even_a_s, even_b_s, e_s);
	}
}

void SketchEncoding::CN_hsort(vec<Lit> &a_s, vec<Lit> &c_s) {
	assert(a_s.size() == c_s.size());

	if (a_s.size() == 2) {
		assert(a_s[0] != a_s[1]);
		vec<Lit> a;
		vec<Lit> b;
		a.push(a_s[0]);
		b.push(a_s[1]);
		CN_hmerge(a, b, c_s);
	} else {

		vec<Lit> upper_a_s;
		vec<Lit> lower_a_s;
		vec<Lit> upper_d_s;
		vec<Lit> lower_d_s;

		for (int i = 0; i < (a_s.size() / 2); i++) {
			lower_a_s.push(a_s[i]);
			upper_d_s.push(mkLit(solver_.nVars(), false));
			solver_.newVar();
			lower_d_s.push(mkLit(solver_.nVars(), false));
			solver_.newVar();
		}

		for (int i = (a_s.size() / 2); i < a_s.size(); i++)
			upper_a_s.push(a_s[i]);

		CN_hsort(lower_a_s, lower_d_s);
		CN_hsort(upper_a_s, upper_d_s);
		CN_hmerge(lower_d_s, upper_d_s, c_s);
	}
}

void SketchEncoding::CN_smerge(vec<Lit> &a_s, vec<Lit> &b_s, vec<Lit> &c_s) {

	assert(a_s.size() == b_s.size());
	assert(a_s.size() + 1 == c_s.size());

	if (a_s.size() == 1) {
		Lit c1 = c_s[0];
		Lit c2 = c_s[1];

		Lit a = a_s[0];
		Lit b = b_s[0];

		addTernaryClause(~a, ~b, c2);
		addBinaryClause(~a, c1);
		addBinaryClause(~b, c1);
	} else {

		vec<Lit> odd_a_s;
		vec<Lit> even_a_s;
		vec<Lit> odd_b_s;
		vec<Lit> even_b_s;

		for (int i = 0; i < a_s.size(); i = i + 2)
			odd_a_s.push(a_s[i]);

		for (int i = 1; i < a_s.size(); i = i + 2)
			even_a_s.push(a_s[i]);

		for (int i = 0; i < b_s.size(); i = i + 2)
			odd_b_s.push(b_s[i]);

		for (int i = 1; i < b_s.size(); i = i + 2)
			even_b_s.push(b_s[i]);

		vec<Lit> d_s;
		vec<Lit> e_s;

		d_s.push(c_s[0]);

		for (int i = 1; i < (a_s.size() / 2) + 1; ++i) {
			d_s.push(mkLit(solver_.nVars(), false));
			solver_.newVar();
		}

		for (int i = 0; i < (a_s.size() / 2) + 1; ++i) {
			e_s.push(mkLit(solver_.nVars(), false));
			solver_.newVar();
		}

		for (int i = 1; i <= (a_s.size() / 2); ++i) {
			addTernaryClause(~d_s[i], ~e_s[i - 1], c_s[2 * i]);
			addBinaryClause(~d_s[i], c_s[2 * i - 1]);
			addBinaryClause(~e_s[i - 1], c_s[2 * i - 1]);
		}

		CN_smerge(odd_a_s, odd_b_s, d_s);
		CN_smerge(even_a_s, even_b_s, e_s);
	}
}

void SketchEncoding::CN_encode(vec<Lit> &a_s, vec<Lit> &c_s, int rhs) {
	assert(a_s.size() % rhs == 0);
	assert(c_s.size() == rhs);

	if (a_s.size() == rhs) {
		CN_hsort(a_s, c_s);
	} else {
		vec<Lit> upper_a_s;
		vec<Lit> lower_a_s;
		vec<Lit> upper_d_s;
		vec<Lit> lower_d_s;

		for (int i = 0; i < rhs; i++) {
			lower_a_s.push(a_s[i]);
			lower_d_s.push(mkLit(solver_.nVars(), false));
			solver_.newVar();
			upper_d_s.push(mkLit(solver_.nVars(), false));
			solver_.newVar();
		}

		for (int i = rhs; i < a_s.size(); i++) {
			upper_a_s.push(a_s[i]);
		}

		vec<Lit> next_c_s;
		for (int i = 0; i < c_s.size(); i++)
			next_c_s.push(c_s[i]);

		next_c_s.push(mkLit(solver_.nVars(), false));
		solver_.newVar();

		CN_encode(lower_a_s, lower_d_s, rhs);
		CN_encode(upper_a_s, upper_d_s, rhs);
		CN_smerge(lower_d_s, upper_d_s, next_c_s);
	}
}

void SketchEncoding::addUnitClause(Lit a) {
	clause_.clear();
	assert(a != lit_Undef);
	assert(var(a) < solver_.nVars());
	clause_.push(a);
	solver_.addClause(clause_);
	clause_.clear();
}

void SketchEncoding::addBinaryClause(Lit a, Lit b) {
	clause_.clear();
	assert(a != lit_Undef && b != lit_Undef);
	assert(var(a) < solver_.nVars() && var(b) < solver_.nVars());
	clause_.push(a);
	clause_.push(b);
	solver_.addClause(clause_);
	clause_.clear();
}

void SketchEncoding::addTernaryClause(Lit a, Lit b, Lit c) {
	clause_.clear();
	assert(a != lit_Undef && b != lit_Undef && c != lit_Undef);
	assert(
			var(a) < solver_.nVars() && var(b) < solver_.nVars()
					&& var(c) < solver_.nVars());
	clause_.push(a);
	clause_.push(b);
	clause_.push(c);
	solver_.addClause(clause_);
	clause_.clear();
}
