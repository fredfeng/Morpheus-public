#include <Predicate.h>
#include <algorithm>

Predicate::Predicate() {
}

Predicate::~Predicate() {
}

bool Predicate::checkTuples(std::vector<std::vector<std::string> > & tuples) {

	bool diff = true;

	for (unsigned i = 0; i < tuples.size(); i++) {
		for (unsigned j = i + 1; j < tuples.size(); j++) {

			unsigned c = 0;
			assert(tuples[i].size() == tuples[j].size());
			for (unsigned z = 0; z < tuples[i].size(); z++) {
				if (!tuples[i][z].compare(tuples[j][z])) {
					c++;
				} else
					break;
			}
			// same
			if (c == tuples[i].size()) {
				diff = false;
				break;
			}
		}
	}

	return diff;
}

bool Predicate::uniqueValues(std::vector<std::vector<std::string> >& matrix,
		int col) {

	if (matrix.empty())
		return true;

	std::set<std::string> values;
	for (unsigned i = 0; i < matrix.size(); i++) {
		//std::cout << "values= " << matrix[i][col] << std::endl;
		values.insert(matrix[i][col]);
	}

	return (values.size() == matrix.size());
}

void Predicate::generatePredicates(Rcpp::DataFrame *df,
		std::vector<std::vector<std::string> > &predicates,
		Component *component, std::string dfname,
		std::vector<std::string>& gather_str,std::set<std::string>& groups) {

	predicates.clear();

	CharacterVector cv = df->names();
	std::vector<std::string> colnames = convert(cv);
//	for (int i = 0; i < colnames.size(); i++)
//		std::cout << "column: " << colnames[i];

// aggregate functions may be updated in the future
	//std::vector<std::string> aggregate_numeric = { "max", "min", "mean", "sum" };
	std::vector<std::string> aggregate_numeric = { "min", "sum", "mean" };
	//std::vector<std::string> aggregate_all = { "n_distinct" };
	std::vector<std::string> aggregate_all;
	std::vector<std::string> aggregate_single = { "n" };

	std::vector<std::vector<std::string>> matrix;
	if (!component->getName().compare("filter")
			|| !component->getName().compare("spread"))
		matrix = Util::convertToMatrix(df);
	//printDataFrame(df);

	// get the types of each column
	std::vector<std::string> coltypes = Util::getColTypes(df, dfname);

	int table = 0;
	int colname = 0;
	int collist = 0;
	int aggregate = 0;
	int predicate = 0;
	int arithmetic = 0;

	for (const auto &key : component->getArgTypes()) {
		if (!key.compare("Table"))
			table++;

		if (!key.compare("ColName"))
			colname++;

		if (!key.compare("Aggregate"))
			aggregate++;

		if (!key.compare("ColList"))
			collist++;

		if (!key.compare("Predicate"))
			predicate++;

		if (!key.compare("Arithmetic"))
			arithmetic++;

	}

	assert(colname <= 2);
	assert(collist <= 1);
	assert(table <= 2);
	assert(aggregate <= 1);
	assert(predicate <= 1);
	assert(arithmetic <= 1);

	if (predicate == 1) {
		// filter

//		std::vector<std::string> op_numeric =
//				{ "<=", "<", "==", "!=", ">", ">=" };
//		std::vector<std::string> op_char = { "==", "!=" };

		std::vector<std::string> op_numeric = { "!=", "==", "<", ">" };
		//std::vector<std::string> op_numeric = { "!=", "==" };
		std::vector<std::string> op_char = { "!=", "==" };

		// a op b, where a=colname ; op=<=,<,=,>,>=,!= ; b=cellvalue
		for (unsigned i = 0; i < colnames.size(); i++) {

			if (!coltypes[i].compare("numeric")) {
				for (unsigned j = 0; j < op_numeric.size(); j++) {
					std::set<std::string> values;

					for (unsigned z = 0; z < matrix.size(); z++) {

						if (values.find(matrix[z][i]) != values.end()
								|| constants_num_.find(stod(matrix[z][i]))
										== constants_num_.end())
							continue;

						std::vector<std::string> pred;
						std::string p = "`" + colnames[i] + "` " + op_numeric[j]
								+ " " + matrix[z][i];
						pred.push_back(p);
						predicates.push_back(pred);

						values.insert(matrix[z][i]);
					}

					// filter compares two columns
//					for (unsigned z = 0; z < colnames.size(); z++) {
//						if (z == i)
//							continue;
//
//						std::vector<std::string> pred;
//						std::string p = "`" + colnames[i] + "` " + op_numeric[j]
//								+ " " + colnames[z];
//						pred.push_back(p);
//						predicates.push_back(pred);
//					}

				}

			}

			if (!coltypes[i].compare("character")) {
				for (unsigned j = 0; j < op_char.size(); j++) {
					std::set<std::string> values;

					for (unsigned z = 0; z < matrix.size(); z++) {

						if (values.find(matrix[z][i]) != values.end()
								|| constants_char_.find(matrix[z][i])
										== constants_char_.end())
							continue;

						std::vector<std::string> pred;
						std::string p = "`" + colnames[i] + "` " + op_char[j]
								+ " " + "\"" + matrix[z][i] + "\"";
						pred.push_back(p);
						predicates.push_back(pred);

						values.insert(matrix[z][i]);
					}

					// filter compares two columns
//					for (unsigned z = 0; z < colnames.size(); z++) {
//						if (z == i)
//							continue;
//
//						std::vector<std::string> pred;
//						std::string p = "`" + colnames[i] + "` " + op_char[j]
//								+ " " + colnames[z];
//						pred.push_back(p);
//						predicates.push_back(pred);
//					}
				}
			}
		}

	} else if (aggregate == 1 && colname == 1) {
		// summarize

		for (unsigned j = 0; j < aggregate_single.size(); j++) {
			std::vector<std::string> pred;
			pred.push_back(aggregate_single[j]);
			pred.push_back("");
			predicates.push_back(pred);
		}

		for (unsigned j = 0; j < colnames.size(); j++) {

			if (groups.find(colnames[j])!=groups.end()){
				continue;
			}

			// only generate predicates for aggregate functions over numeric types
			if (!coltypes[j].compare("numeric")) {

				for (unsigned i = 0; i < aggregate_numeric.size(); i++) {
					std::vector<std::string> pred;
					pred.push_back(aggregate_numeric[i]);
					pred.push_back(colnames[j]);
					predicates.push_back(pred);
				}
			}

			for (unsigned i = 0; i < aggregate_all.size(); i++) {
				std::vector<std::string> pred;
				pred.push_back(aggregate_all[i]);
				pred.push_back(colnames[j]);
				predicates.push_back(pred);
			}
		}

	} else if (colname == 1) {
		// separate

		for (unsigned i = 0; i < colnames.size(); i++) {
			std::vector<std::string> pred;
			pred.push_back(colnames[i]);
			predicates.push_back(pred);
		}

	} else if (colname == 2) {
		// spread, unite

		if (!component->getName().compare("spread")) {

			// pick the value column
			for (unsigned i = 0; i < colnames.size(); i++) {

				// generate tuples
				std::vector<std::vector<std::string> > tuples;
				for (unsigned c = 0; c < matrix.size(); c++) {
					std::vector<std::string> t;
					for (unsigned w = 0; w < colnames.size(); w++) {
						if (w == i)
							continue;

						t.push_back(matrix[c][w]);
					}
//					std::cout << "Tuple= ";
//					for (int x = 0 ; x < t.size(); x++){
//						std::cout << t[x]  << " ";
//					}
//					std::cout << std::endl;
					tuples.push_back(t);
				}

				if (!checkTuples(tuples)) {
					continue;
				}

				// pick the key column
				for (unsigned j = 0; j < colnames.size(); j++) {

					if (i == j)
						continue;

					//std::vector<std::string> pred1;
					std::vector<std::string> pred2;

//					pred1.push_back(colnames[i]);
//					pred1.push_back(colnames[j]);
//					predicates.push_back(pred1);

					pred2.push_back(colnames[j]);
					pred2.push_back(colnames[i]);
					predicates.push_back(pred2);

				}
			}

		} else {

			// generate all pairs (i,j) and (j,i) with i!=j
			for (unsigned i = 0; i < colnames.size(); i++) {
				for (unsigned j = i + 1; j < colnames.size(); j++) {

					std::vector<std::string> pred1;
					std::vector<std::string> pred2;

					pred1.push_back(colnames[i]);
					pred1.push_back(colnames[j]);
					predicates.push_back(pred1);

					pred2.push_back(colnames[j]);
					pred2.push_back(colnames[i]);
					predicates.push_back(pred2);
				}
			}
		}

	} else if (collist == 1) {
		// gather, group_by, select

		// order matters since we will generate tables with different column order
		// can we just fix the order at the end and not consider all permutations?

		// FIXME: change the permutation generation to reverse the order
		std::vector<std::vector<std::string>> tmp;

		std::string s1 = "";
		std::string s2 = "";

		//gather_str.clear();

		if (!component->getName().compare("gather")) {
			s1 = Util::getColName();
			s2 = Util::getColName();

			gather_str.push_back(s1);
			gather_str.push_back(s2);
			assert(gather_str.size() == 2 || gather_str.size() == 4);
		}

		unsigned n = colnames.size();
		for (unsigned r = 1; r <= n; r++) {

			std::vector<bool> v(n);
			std::fill(v.begin(), v.begin() + r, true);

			do {
				std::vector<std::string> pred;
				for (unsigned i = 0; i < n; ++i) {
					if (v[i]) {
						pred.push_back(colnames[i]);
					}
				}

				if (!component->getName().compare("group_by")) {
					if (pred.size() <= 2)
						predicates.push_back(pred);
				} else {
					if (pred.size() <= 2) {

						if (!component->getName().compare("gather")) {

							std::vector<std::string> pred2;
							if ((n == 4 && pred.size() == 1) || (n > 4)) {
								// consider the negative predicates

								for (unsigned i = 0; i < pred.size(); i++) {
									std::string neg = "-" + pred[i];
									pred2.push_back(neg);
								}
							}

							std::vector<std::string> p1(pred.begin(),
									pred.end());

							if (pred.size() > 1) {
								p1.push_back(s1);
								p1.push_back(s2);

								predicates.push_back(p1);
							}

							std::vector<std::string> p2(pred2.begin(),
									pred2.end());

							if (!pred2.empty()) {
								p2.push_back(s1);
								p2.push_back(s2);
								predicates.push_back(p2);
							}

							if (gather_str.size() > 2) {

								if (std::find(colnames.begin(), colnames.end(),
										gather_str[1]) == colnames.end()) {

									std::vector<std::string> p1(pred.begin(),
											pred.end());
									std::vector<std::string> p2(pred2.begin(),
											pred2.end());

									if (pred.size() > 1) {
										p1.push_back(s1);
										p1.push_back(gather_str[1]);
										predicates.push_back(p1);
									}

									if (!pred2.empty()) {
										p2.push_back(s1);
										p2.push_back(gather_str[1]);
										predicates.push_back(p2);
									}
								}

								if (std::find(colnames.begin(), colnames.end(),
										gather_str[0]) == colnames.end()) {

									std::vector<std::string> p3(pred.begin(),
											pred.end());
									std::vector<std::string> p4(pred2.begin(),
											pred2.end());


									if (pred.size() > 1) {
										p3.push_back(gather_str[0]);
										p3.push_back(s2);
										predicates.push_back(p3);
									}

									if (!pred2.empty()) {
										p4.push_back(gather_str[0]);
										p4.push_back(s2);
										predicates.push_back(p4);
									}
								}
							}

						} else {

							predicates.push_back(pred);

							if ((n == 4 && pred.size() == 1) || (n > 4)) {
								// consider the negative predicates
								std::vector<std::string> pred2;
								for (unsigned i = 0; i < pred.size(); i++) {
									std::string neg = "-" + pred[i];
									pred2.push_back(neg);
								}
								predicates.push_back(pred2);
							}
						}
					}
				}

			} while (std::prev_permutation(v.begin(), v.end()));
		}

		if (n > 2) {
			std::vector<std::string> pred;
			for (unsigned i = 0; i < n; ++i) {
				pred.push_back(colnames[i]);
			}

			if (!component->getName().compare("gather")) {
				std::vector<std::string> p1(pred.begin(), pred.end());

				// normal solution
				p1.push_back(s1);
				p1.push_back(s2);
				predicates.push_back(p1);

				if (gather_str.size() > 2) {

					if (std::find(colnames.begin(), colnames.end(),
							gather_str[1]) == colnames.end()) {
						std::vector<std::string> p1(pred.begin(), pred.end());

						p1.push_back(s1);
						p1.push_back(gather_str[1]);
						predicates.push_back(p1);
					}

					if (std::find(colnames.begin(), colnames.end(),
							gather_str[0]) == colnames.end()) {
						std::vector<std::string> p3(pred.begin(), pred.end());

						p3.push_back(gather_str[0]);
						p3.push_back(s2);
						predicates.push_back(p1);
					}

				}
			} else {
				if (component->getName().compare("group_by") != 0)
					predicates.push_back(pred);
			}
		}

//		for (int i = tmp.size()-1; i >= 0; i--)
//			predicates.push_back(tmp[i]);

	} else if (arithmetic == 1) {
		// mutate

		// a op b , a = colname, op = +,-,*,/ b = colname
		//std::vector<std::string> op_arithmetic = { "*", "-", "+", "/" };
		//std::vector<std::string> op_arithmetic = { "/", "-" };
		std::vector<std::string> op_arithmetic = { "/" };

		for (unsigned i = 0; i < colnames.size(); i++) {
			if (!coltypes[i].compare("numeric")) {
				std::vector<std::string> pred;
				std::string s = colnames[i] + " / " + "sum(" + colnames[i]
						+ ")";
				pred.push_back(s);
				predicates.push_back(pred);
			}
		}

		for (unsigned i = 0; i < colnames.size(); i++) {
			for (unsigned j = 0; j < colnames.size(); j++) {
				if (i == j)
					continue;

//				std::cout << "coltype,i=" << coltypes[i] << std::endl;
//				std::cout << "coltype,j=" << coltypes[j] << std::endl;

				if (!coltypes[i].compare("numeric")
						&& !coltypes[j].compare("numeric")) {

					for (unsigned z = 0; z < op_arithmetic.size(); z++) {

						std::vector<std::string> pred1;
						std::string s = colnames[i] + " " + op_arithmetic[z]
								+ " " + colnames[j];
						pred1.push_back(s);
						predicates.push_back(pred1);

//						if (!op_arithmetic[z].compare("*")
//								|| !op_arithmetic[z].compare("+")) {
//							std::vector<std::string> pred2;
//							std::string s = colnames[j] + " " + op_arithmetic[z]
//									+ " " + colnames[i];
//							pred2.push_back(s);
//							predicates.push_back(pred2);
//						}
					}
				}
			}
		}
	}

}

std::vector<std::vector<std::string>> Predicate::getValues(Rcpp::DataFrame *df,
		std::string name, std::set<std::string> &values_char,
		std::set<double> &values_num) {

	CharacterVector cv = df->names();
	std::vector<std::string> colnames = convert(cv);
	std::vector<std::string> coltypes;

	for (unsigned j = 0; j < colnames.size(); j++) {
		std::string s = "class(" + name + "$" + "\"" + colnames[j] + "\"" + ")";
		Rcpp::ExpressionVector rv(s);
		CharacterVector output = rv.eval();
		std::vector<std::string> r_type = convert(output);
		assert(r_type.size() == 1);
		assert(
				!r_type[0].compare("numeric")
						|| !r_type[0].compare("character"));
		coltypes.push_back(r_type[0]);
	}

	std::vector<std::vector<std::string>> matrix = Util::convertToMatrix(df);
	if (matrix.size() > 0) {
		for (unsigned y = 0; y < matrix[0].size(); y++) {
			for (unsigned x = 0; x < matrix.size(); x++) {
				if (!coltypes[y].compare("numeric"))
					values_num.insert(stod(matrix[x][y]));
				else
					values_char.insert(matrix[x][y]);
			}
		}
	}

	return matrix;

}

