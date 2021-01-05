#ifndef MODEL_PREDICATE_H_
#define MODEL_PREDICATE_H_

#include <RInside.h>
#include <vector>
#include <iostream>
#include <string>
#include <Component.h>
#include <Util.h>
#include <set>

using namespace Rcpp;

class Predicate {

public:
	Predicate();
	~Predicate();

	bool checkTuples(std::vector<std::vector<std::string> > & tuples);

	void generatePredicates(Rcpp::DataFrame *df,
			std::vector<std::vector<std::string> > &predicates,
			Component *component, std::string dfname, std::vector<std::string>& gather_str,std::set<std::string>& groups);

	void printDataFrame(Rcpp::DataFrame *df) {
		std::vector<std::vector<std::string>> matrix = Util::convertToMatrix(df);
		for (unsigned i = 0; i < matrix.size(); i++) {
			for (unsigned j = 0; j < matrix[i].size(); j++) {
				std::cout << "[" << matrix[i][j] << "]";
			}
			std::cout << std::endl;
		}
	}

	bool uniqueValues(std::vector<std::vector<std::string> >& matrix, int col);

	bool checkColumns(std::vector<Rcpp::DataFrame> &df, int index,
			std::string &label, std::string &code, RInside &R) {

		if (df[index].nrows() == 0) {
			//std::cout << "Dataframe is empty" << std::endl;
			return false;
		}
		//assert (!code.empty());

		/*
		 Rcpp::CharacterVector cv = df[index].names();
		 std::vector<std::string> colnames = convert(cv);

		 for (unsigned i = 0; i < colnames.size(); i++) {
		 if (Util::is_number(colnames[i])) {

		 std::string rename = "colnames(" + label + ")[" + std::to_string(i+1) + "]=\"XXMORPHXX" + colnames[i] + "\"";
		 R.parseEval(rename);
		 //std::cout << "Renaming: " << rename << std::endl;
		 code.append("\n" + rename);
		 //return false;
		 }
		 }

		 df[index] = R.parseEval(label);
		 */

		return true;
	}

	std::vector<std::vector<std::string>> getValues(Rcpp::DataFrame *df,
			std::string name, std::set<std::string> &values_char,
			std::set<double> &values_num);

	void generateInputConstant(std::vector<Rcpp::DataFrame> &df,
			std::vector<std::string> names) {

		for (unsigned i = 0; i < df.size(); i++) {
			CharacterVector cv = df[i].names();
			std::vector<std::string> colnames = convert(cv);
			std::vector<std::string> coltypes;

			for (unsigned j = 0; j < colnames.size(); j++) {
				std::string s = "class(" + names[i] + "$" + "\"" + colnames[j]
						+ "\"" + ")";
				Rcpp::ExpressionVector rv(s);
				CharacterVector output = rv.eval();
				std::vector<std::string> r_type = convert(output);
				assert(r_type.size() == 1);
				assert(
						!r_type[0].compare("numeric")
								|| !r_type[0].compare("character"));
				coltypes.push_back(r_type[0]);
				constants_char_.insert(colnames[j]);
			}

			std::vector<std::vector<std::string>> matrix = Util::convertToMatrix(
					&df[i]);
			assert(matrix.size() > 0);
			for (unsigned y = 0; y < matrix[0].size(); y++) {
				for (unsigned x = 0; x < matrix.size(); x++) {
					if (!coltypes[y].compare("numeric"))
						constants_num_.insert(stod(matrix[x][y]));
					else
						constants_char_.insert(matrix[x][y]);
				}
			}
		}


//			for (std::set<double>::iterator it=constants_num_.begin(); it!=constants_num_.end(); ++it){
//				std::cout << "Num=" << *it << std::endl;
//			}
//
//			for (std::set<std::string>::iterator it=constants_char_.begin(); it!=constants_char_.end(); ++it){
//				std::cout << "Char=" << *it << std::endl;
//			}

	}

protected:

	std::set<std::string> constants_char_;
	std::set<double> constants_num_;

	std::vector<std::string> convert(Rcpp::CharacterVector &f) {
		std::vector<std::string> s(f.size());
		for (unsigned i = 0; i < f.size(); i++) {
			s[i] = std::string(f[i]);
		}
		return (s);
	}

};

#endif
