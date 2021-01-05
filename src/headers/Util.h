/*
 * Util.h
 *
 *  Created on: Aug 31, 2016
 *      Author: yu
 */

#ifndef UTIL_UTIL_H_
#define UTIL_UTIL_H_

#include <string>
#include <vector>
#include <unordered_set>
#include <unordered_map>
#include"z3++.h"
#include <sstream> //stringstream
#include <RInside.h>
#include <algorithm>
#include <iterator>
#include <SketchTree.h>
#include <cmath>
#include <ctime>// include this header

using namespace z3;

static int counter_ = 1;

static int str_counter_ = 1;

static int var_counter_ = 1;

static std::unordered_map<std::string, int> str_int_map_;

static std::unordered_map<int, std::string> int_str_map_;

#define MK_EXPR1(_fn, _arg)                     \
  Z3_ast r = _fn(_arg.ctx(), _arg);           \
  _arg.check_error();                         \
  return expr(_arg.ctx(), r);

#define MK_EXPR2(_fn, _arg1, _arg2)             \
  check_context(_arg1, _arg2);                \
  Z3_ast r = _fn(_arg1.ctx(), _arg1, _arg2);  \
  _arg1.check_error();                        \
  return expr(_arg1.ctx(), r);

class Util {

public:
	Util();

	static void trimDecimal(std::string& toFormat) {
		while (((toFormat.find(".") != std::string::npos)
				&& (toFormat.substr(toFormat.length() - 1, 1) == "0"))
				|| (toFormat.substr(toFormat.length() - 1, 1) == ".")) {
			toFormat.pop_back();
		}
	}

	// Dataframe utilities
	static std::vector<std::multiset<double>> getNumColValues(
			Rcpp::DataFrame *df) {

		assert(df != NULL);

		// assumes that all numeric values are not numbers
		std::vector<std::multiset<double>> values;

		//printDataFrame(df);

		for (unsigned i = 0; i < df->size(); i++) {

			std::multiset<double> cvalues;
			if (TYPEOF((*df)[i]) == REALSXP) {

				Rcpp::NumericVector tmp = Rcpp::as<Rcpp::NumericVector>(
						(*df)[i]);
				for (unsigned j = 0; j < tmp.size(); j++) {
					if (std::isnan(tmp[j])) {
						values.clear();
						return values;
					} else {
						cvalues.insert(tmp[j]);
					}
				}

			} else if (TYPEOF((*df)[i]) == INTSXP) {

				Rcpp::IntegerVector tmp = Rcpp::as<Rcpp::IntegerVector>(
						(*df)[i]);
				for (unsigned j = 0; j < tmp.size(); j++) {
					if (std::isnan(tmp[j])) {
						values.clear();
						return values;
					} else {
						cvalues.insert(tmp[j]);
					}
				}

			}
			if (!cvalues.empty())
				values.push_back(cvalues);
		}

		return values;
	}

	static std::vector<std::multiset<std::string>> getCharColValues(
			Rcpp::DataFrame *df) {

		assert(df != NULL);
		std::vector<std::multiset<std::string>> values;

		for (unsigned i = 0; i < df->size(); i++) {

			std::multiset<std::string> cvalues;
			if (TYPEOF((*df)[i]) == STRSXP) {

				Rcpp::CharacterVector tmp = Rcpp::as<Rcpp::CharacterVector>(
						(*df)[i]);
				for (unsigned j = 0; j < tmp.size(); j++) {
					cvalues.insert(std::string(tmp[j]));
				}

			}
			if (!cvalues.empty())
				values.push_back(cvalues);
		}
		return values;
	}

	static std::unordered_set<int> getValues(Rcpp::DataFrame *df) {

		std::unordered_set<int> bag;
		for (unsigned i = 0; i < df->size(); i++) {

			if (TYPEOF((*df)[i]) == REALSXP) {

				Rcpp::NumericVector tmp = Rcpp::as<Rcpp::NumericVector>(
						(*df)[i]);
				for (unsigned j = 0; j < tmp.size(); j++) {
					if (std::isnan(tmp[j])) {
						bag.insert(getIntByStr("NA"));
					} else {
						double value = tmp[j];
						std::string strId = std::to_string(value);
						//FIXME:ugly.
						if (Util::contains(strId, ".0000")) {
							trimDecimal(strId);
						}
						int key = getIntByStr(strId);
						bag.insert(key);
					}
				}

			} else if (TYPEOF((*df)[i]) == INTSXP) {

				Rcpp::IntegerVector tmp = Rcpp::as<Rcpp::IntegerVector>(
						(*df)[i]);
				for (unsigned j = 0; j < tmp.size(); j++) {
					if (std::isnan(tmp[j])) {
						bag.insert(getIntByStr("NA"));
					} else {
						bag.insert(getIntByStr(std::to_string(tmp[j])));
					}
				}

			} else if (TYPEOF((*df)[i]) == STRSXP) {

				Rcpp::CharacterVector tmp = Rcpp::as<Rcpp::CharacterVector>(
						(*df)[i]);
				for (unsigned j = 0; j < tmp.size(); j++) {
//					std::cout << "string type:" << tmp[i] << std::endl;
					bag.insert(getIntByStr(std::string(tmp[j])));
				}

			} else {
				assert(false);
			}
		}

		Rcpp::CharacterVector cv = df->names();
		for (int i = 0; i < cv.size(); i++) {
			std::string s = std::string(cv[i]);
			bag.insert(getIntByStr(s));
		}

		return bag;
	}

	static std::string dataFrameToString(Rcpp::DataFrame * df) {

		std::string result = "";
		std::vector<std::vector<std::string>> matrix = convertToMatrix(df);

		if (matrix.empty()) {
			result += "MMEMPTY";
		} else {
			for (unsigned i = 0; i < matrix.size(); i++) {
				for (unsigned j = 0; j < matrix[i].size(); j++) {
					result += matrix[i][j];
					result += "MM";
				}
				result += "MM";
			}
		}

		Rcpp::CharacterVector cv = df->names();
		for (int i = 0; i < cv.size(); i++) {
			result += std::string(cv[i]);
		}

		return result;
	}

	// Same performance as previous version
	static std::unordered_set<int> getValuesV2(Rcpp::DataFrame * df) {

		assert(df != NULL);

		int columns = df->size();
		int rows = df->nrows();

		std::unordered_set<int> bag;

		// cell contents
		for (int i = 0; i < columns; ++i) {
			SEXP element = VECTOR_ELT(df->get__(), i);

			for (int j = 0; j < rows; ++j) {
				switch (TYPEOF(element)) {
				case REALSXP:
					if (std::isnan(REAL(element)[j])) {
						bag.insert(getIntByStr("NA"));
					} else {
						double value = REAL(element)[j];
						std::string strId = std::to_string(value);
						if (Util::contains(strId, ".0000")) {
							trimDecimal(strId);
						}
						int key = getIntByStr(strId);
						bag.insert(key);
					}
					break;
				case INTSXP:
				case LGLSXP:
					if (std::isnan(INTEGER(element)[j])) {
						bag.insert(getIntByStr("NA"));
					} else {
						bag.insert(
								getIntByStr(
										std::to_string(INTEGER(element)[j])));
					}
					break;
				case STRSXP:
					bag.insert(
							getIntByStr(
									std::string(CHAR(STRING_ELT(element, j)))));
					break;
				}
			}
		}

		// column names
		Rcpp::CharacterVector cv = df->names();
		for (int i = 0; i < cv.size(); i++) {
			std::string s = std::string(cv[i]);
			bag.insert(getIntByStr(s));
		}

		return bag;
	}

	static std::vector<std::vector<std::string>> convertToMatrix(
			Rcpp::DataFrame * df) {

		std::vector<std::vector<std::string>> matrix;
		std::vector<std::vector<std::string>> transpose;

		for (unsigned i = 0; i < df->size(); i++) {
			std::vector<std::string> columns;

			if (TYPEOF((*df)[i]) == REALSXP) {

				Rcpp::NumericVector tmp = Rcpp::as<Rcpp::NumericVector>(
						(*df)[i]);
				for (unsigned j = 0; j < tmp.size(); j++) {
					if (std::isnan(tmp[j])) {
						columns.push_back("NA");
					} else {
						columns.push_back(std::to_string(tmp[j]));
					}
				}

			} else if (TYPEOF((*df)[i]) == INTSXP) {

				Rcpp::IntegerVector tmp = Rcpp::as<Rcpp::IntegerVector>(
						(*df)[i]);
				for (unsigned j = 0; j < tmp.size(); j++) {
					if (std::isnan(tmp[j])) {
						columns.push_back("NA");
					} else {
						columns.push_back(std::to_string(tmp[j]));
					}
				}

			} else if (TYPEOF((*df)[i]) == STRSXP) {

				Rcpp::CharacterVector tmp = Rcpp::as<Rcpp::CharacterVector>(
						(*df)[i]);
				for (unsigned j = 0; j < tmp.size(); j++) {
					columns.push_back(std::string(tmp[j]));
				}

			} else
				assert(false);
			matrix.push_back(columns);
		}

		if (!matrix.empty()) {

			for (unsigned i = 0; i < matrix[0].size(); i++) {
				std::vector<std::string> rows;
				for (unsigned j = 0; j < matrix.size(); j++) {
					rows.push_back(matrix[j][i]);
				}
				transpose.push_back(rows);
			}
		}

		return transpose;
	}

	static std::vector<std::string> getColTypes(Rcpp::DataFrame *df,
			std::string name) {

		std::vector<std::string> coltypes;

		for (unsigned i = 0; i < df->size(); i++) {

			if (TYPEOF((*df)[i]) == REALSXP) {
				coltypes.push_back("numeric");

			} else if (TYPEOF((*df)[i]) == INTSXP) {
				coltypes.push_back("numeric");

			} else if (TYPEOF((*df)[i]) == STRSXP) {
				coltypes.push_back("character");

			} else
				assert(false);
		}

//		Rcpp::CharacterVector cv = df->names();
//		std::vector<std::string> colnames(cv.size());
//		for (unsigned i = 0; i < cv.size(); i++) {
//			colnames[i] = std::string(cv[i]);
//		}
//
//		for (unsigned i = 0; i < colnames.size(); i++) {
//			std::string s = "class(" + name + "$" + "\"" + colnames[i] + "\""
//					+ ")";
//			Rcpp::ExpressionVector rv(s);
//			Rcpp::CharacterVector output = rv.eval();
//			std::vector<std::string> r_type(output.size());
//			for (unsigned j = 0; j < output.size(); j++) {
//				r_type[j] = std::string(output[j]);
//			}
//			assert(r_type.size() == 1);
//			assert(
//					!r_type[0].compare("numeric")
//							|| !r_type[0].compare("integer")
//							|| !r_type[0].compare("character"));
//			if (!r_type[0].compare("integer"))
//				coltypes.push_back("numeric");
//			else
//				coltypes.push_back(r_type[0]);
//		}

		return coltypes;

	}

	static void printAllDataFrames(Node * root) {

		while (root != NULL) {
			std::cout << "node id =" << root->id << std::endl;
			Util::printDataFrame(root->dataframe);
			root = root->left;
		}
	}

	static void printDataFrame(Rcpp::DataFrame *df) {

		if (df == NULL) {
			std::cout << "Data frame is null" << std::endl;
			return;
		}

		std::cout << "columns= " << df->size() << " rows= " << df->nrows()
				<< std::endl;

		if (df->size() > 0) {

			std::vector<std::vector<std::string> > matrix;

			for (int i = 0; i < df->size(); i++) {

				Rcpp::CharacterVector cv = df->names();
				std::vector<std::string> column;
				column.push_back(std::string(cv[i]));

				if (TYPEOF((*df)[i]) == REALSXP) {

					Rcpp::NumericVector tmp = Rcpp::as<Rcpp::NumericVector>(
							(*df)[i]);
					for (unsigned i = 0; i < tmp.size(); i++) {
						if (std::isnan(tmp[i])) {
							column.push_back("NA");
						} else {
							double value = tmp[i];
							std::string strId = std::to_string(value);
							//FIXME:ugly.
							if (Util::contains(strId, ".0000")) {
								trimDecimal(strId);
							}

							column.push_back(strId);
						}
					}

				} else if (TYPEOF((*df)[i]) == INTSXP) {

					Rcpp::IntegerVector tmp = Rcpp::as<Rcpp::IntegerVector>(
							(*df)[i]);
					for (unsigned i = 0; i < tmp.size(); i++) {
						if (std::isnan(tmp[i])) {
							column.push_back("NA");
						} else {
							column.push_back(std::to_string(tmp[i]));
						}
					}

				} else if (TYPEOF((*df)[i]) == STRSXP) {

					Rcpp::CharacterVector tmp = Rcpp::as<Rcpp::CharacterVector>(
							(*df)[i]);
					for (unsigned i = 0; i < tmp.size(); i++) {
						column.push_back(std::string(tmp[i]));
					}

				} else {
					assert(false);
				}
				matrix.push_back(column);
			}

			std::vector<std::vector<std::string>> transpose;
			if (!matrix.empty()) {

				for (unsigned i = 0; i < matrix[0].size(); i++) {
					std::vector<std::string> rows;
					for (unsigned j = 0; j < matrix.size(); j++) {
						rows.push_back(matrix[j][i]);
					}
					transpose.push_back(rows);
				}
			}

			for (unsigned i = 0; i < transpose.size(); i++) {
				for (unsigned j = 0; j < transpose[i].size(); j++) {
					std::cout << "[" << transpose[i][j] << "]";
				}
				std::cout << std::endl;
			}

		}
	}

	static int getIntByStr(std::string str) {
		int val = 0;
		if (str_int_map_.find(str) == str_int_map_.end()) {
			str_int_map_[str] = str_counter_;
			int_str_map_[str_counter_] = str;
			val = str_counter_;
			str_counter_++;
		} else {
			val = str_int_map_[str];
		}
		return val;
	}

	static std::vector<int> getHeader(Rcpp::DataFrame *df) {
		std::vector<int> vec;
		Rcpp::CharacterVector cv = df->names();
		for (int i = 0; i < cv.size(); i++) {
			std::string s = std::string(cv[i]);
			vec.push_back(getIntByStr(s));
		}
		return vec;
	}

	static std::unordered_set<int> getHeaderSet(Rcpp::DataFrame *df) {
		std::unordered_set<int> bag;
		Rcpp::CharacterVector cv = df->names();
		for (int i = 0; i < cv.size(); i++) {
			std::string s = std::string(cv[i]);
			bag.insert(getIntByStr(s));
		}
		return bag;
	}

	// Debugging messages
	static inline void message(int verbosity, const char* format, ...) {
		if (verbosity <= 0) {
			va_list args;
			va_start(args, format);
			vprintf(format, args);
			va_end(args);
			printf("\n");
			fflush(stdout);
		}
	}

	//////////////////////String operations////////////////////////////
	static void split(const std::string &s, char delim, std::vector<std::string> &elems) {
	    std::stringstream ss;
	    ss.str(s);
	    std::string item;
	    while (std::getline(ss, item, delim)) {
	        elems.push_back(item);
	    }
	}

	static bool replace(std::string& str, const std::string& from,
			const std::string& to) {
		size_t start_pos = str.find(from);
		if (start_pos == std::string::npos)
			return false;
		str.replace(start_pos, from.length(), to);
		return true;
	}

	static void replaceAll(std::string& str, const std::string& from,
			const std::string& to) {
		if (from.empty())
			return;
		size_t start_pos = 0;
		while ((start_pos = str.find(from, start_pos)) != std::string::npos) {
			str.replace(start_pos, from.length(), to);
			start_pos += to.length(); // In case 'to' contains 'from', like replacing 'x' with 'yx'
		}
	}

	// s1 contains s2?
	static bool contains(std::string s1, std::string s2) {
		return (s1.find(s2) != std::string::npos);
	}

	static std::string append(std::string s1, int i) {
		std::ostringstream s;
		s << s1 << i;
		return s.str();
	}

	// trim from start
	static inline std::string &ltrim(std::string &s) {
		s.erase(s.begin(),
				std::find_if(s.begin(), s.end(),
						std::not1(std::ptr_fun<int, int>(std::isspace))));
		return s;
	}

	// trim from end
	static inline std::string &rtrim(std::string &s) {
		s.erase(
				std::find_if(s.rbegin(), s.rend(),
						std::not1(std::ptr_fun<int, int>(std::isspace))).base(),
				s.end());
		return s;
	}

	// trim from both ends
	static inline std::string &trim(std::string &s) {
		return ltrim(rtrim(s));
	}

	static inline unsigned int DJBHash(const std::string& str) {
		unsigned int hash = 5381;

		for (std::size_t i = 0; i < str.length(); i++) {
			hash = ((hash << 5) + hash) + str[i];
		}

		return hash;
	}

	////////////////////Z3 operations////////////////////////////////////
	static z3::expr* convertStrToExpr(z3::context& c, std::string cst) {
		Z3_ast ast = Z3_parse_smtlib2_string(c, cst.c_str(), 0, 0, 0, 0, 0, 0);
		z3::expr* e = new z3::expr(c, ast);
		return e;
	}

	static z3::expr mk_or(z3::expr_vector args) {
		if (args.size() == 1) {
			return args[0];
		} else {
			std::vector<Z3_ast> array;
			for (unsigned i = 0; i < args.size(); i++)
				array.push_back(args[i]);
			return to_expr(args.ctx(),
					Z3_mk_or(args.ctx(), array.size(), &(array[0])));
		}
	}

	static z3::expr mk_xor(z3::expr_vector args) {
		expr res = args[0];
		if (args.size() > 1) {
			for (unsigned i = 1; i < args.size(); i++)
				res = to_expr(res.ctx(), Z3_mk_xor(res.ctx(), res, args[i]));
		}
		return res;
	}

	static z3::expr mk_and(z3::expr_vector args) {
		if (args.size() == 1) {
			return args[0];
		} else {
			std::vector<Z3_ast> array;
			for (unsigned i = 0; i < args.size(); i++)
				array.push_back(args[i]);
			return to_expr(args.ctx(),
					Z3_mk_and(args.ctx(), array.size(), &(array[0])));
		}
	}

	/**
	 * type: 0: input   1: output
	 * index: index of table. Here we could have multiple input tables
	 * Use 99 as the id for input and output.
	 *
	 */
	static z3::expr getDataFrameExpr(z3::context& ctx, RInside& R,
			std::string dfOut, std::vector<std::string> dfIns, int type, int index,
			bool multi) {
		Rcpp::DataFrame ds = R.parseEval(dfOut);
		if (type == 0) {
			ds = R.parseEval(dfIns[0]);
		}

		//input or output?
		std::string prefix = (type == 0 ? "_IN_99_" : "_OUT_99_");
		std::string rowCstName = Util::append("ROW" + prefix, index);
		std::string colCstName = Util::append("COL" + prefix, index);
		int rowNum = ds.nrows();
		int colNum = ds.size();
		expr rowCst = ctx.int_const(rowCstName.c_str());
		expr colCst = ctx.int_const(colCstName.c_str());

		//set constraint.
		std::string setCstName = Util::append("SET" + prefix, index);
		std::vector<int> vals = getHeader(&ds);
		expr s1 = Util::empty_set(get_set_sort(ctx));
		expr set_cst = set_expr(ctx, setCstName.c_str());

		for (unsigned i = 0; i < vals.size(); i++) {
			int v = vals[i];
			s1 = Util::set_add(s1, ctx.int_val(v));
		}

//		expr cst = ((rowCst == rowNum) && (colCst == colNum) && (set_cst == s1));
		expr cst = ((rowCst == rowNum) && (colCst == colNum));

		//spec 4:
		expr extra = ctx.int_const("dummy");
		std::string headCstName = Util::append("HEAD" + prefix, index);
		std::string contentCstName = Util::append("CONTENT" + prefix, index);
		std::string groupCstName = Util::append("GROUP" + prefix, index);
		expr headCst = ctx.int_const(headCstName.c_str());
		expr contentCst = ctx.int_const(contentCstName.c_str());
		expr groupCst = ctx.int_const(groupCstName.c_str());
		///additional input
		if (type == 0) {
			extra = (headCst == 0) && (contentCst == 0) && (groupCst == 1);
			///additional output
		} else {
			//compute difference.
			Rcpp::DataFrame dsOut = R.parseEval(dfOut);
			std::vector<Rcpp::DataFrame> inputVec;
			for(unsigned j = 0; j < dfIns.size(); j++) {
				Rcpp::DataFrame dsIn = R.parseEval(dfIns[j]);
				inputVec.push_back(dsIn);
			}
//			std::cout << "input-output content:" << contentCstName << std::endl;
			expr e4 = Util::getInOutConstraint(ctx, dsOut, inputVec,
					headCstName, contentCstName, multi);
			//content
			//don't know the number of final group
			expr unknowGroup = ctx.int_const("unknown_group");
			extra = e4 && (groupCst == unknowGroup);
		}

		cst = cst && extra;
		return cst;
	}

	static z3::expr getInOutConstraint(z3::context& ctx, Rcpp::DataFrame dsOut,
			std::vector<Rcpp::DataFrame> inputVec, std::string headCstName,
			std::string contentCstName, bool multi) {
		expr headCst = ctx.int_const(headCstName.c_str());
		expr contentCst = ctx.int_const(contentCstName.c_str());

//		Rcpp::DataFrame dsIn = inputVec[0];
//		std::cout << "output table:\n";
//		Util::printDataFrame(&dsOut);
		//header
		std::unordered_set<int> outContent = getValues(&dsOut);
		std::unordered_set<int> outHead = getHeaderSet(&dsOut);
		std::unordered_set<int> diffHead = outHead;
		std::unordered_set<int> diffContent;
		for(auto in : inputVec) {
//			std::cout << "input table:\n";
//			Util::printDataFrame(&in);
			std::unordered_set<int> inHead = getHeaderSet(&in);
			std::unordered_set<int> inContent = getValues(&in);
			diffHead = setDiff(diffHead, inHead);
			diffHead = setDiff(diffHead, inContent);
			diffContent = setDiff(outContent, inContent);
		}

//		std::unordered_set<int> inHead = getHeaderSet(&dsIn);
//		std::unordered_set<int> diffHead = setDiff(outHead, inHead);
//		std::unordered_set<int> inContent = getValues(&dsIn);
//		diffHead = setDiff(diffHead, inContent);
//		std::unordered_set<int> diffContent = setDiff(outContent, inContent);
//		for(auto f : diffContent) {
//			std::cout << int_str_map_[f] <<",";
//		}
//		std::cout << "\n------------input data....";
//		for(auto f : inContent) {
//			std::cout << int_str_map_[f] <<",";
//		}
//		std::cout << "\n-------------output data...";
//		for(auto f : outContent) {
//			std::cout << int_str_map_[f] <<",";
//		}
//		std::cout << "\n";

		//content
		//don't know the number of final group
		int i1 = diffHead.size();
		int i2 = diffContent.size();
		z3::expr extra = (headCst == i1) && (contentCst == i2);

		//FIXME.
		if (multi)
			extra = (headCst == i1);
		return extra;
	}

	/**
	 * Generate constraint for a given dataframe.
	 * Id is in the form of: IN_99_0 or 17_0;
	 */
	static z3::expr* getDataFrameExpr(z3::context& ctx, Rcpp::DataFrame ds,
			std::string id, std::vector<Rcpp::DataFrame> inputVec) {
		std::string rowCstName = "ROW_" + id;
		std::string colCstName = "COL_" + id;
		int rowNum = ds.nrows();
		int colNum = ds.size();
		z3::expr rowCst = ctx.int_const(rowCstName.c_str());
		z3::expr colCst = ctx.int_const(colCstName.c_str());

		//set constraint.
//		std::string setCstName = "SET_" + id;
//		std::vector<int> vals = getHeader(&ds);
//		expr s1 = Util::empty_set(get_set_sort(ctx));
//		expr set_cst = set_expr(ctx, setCstName.c_str());
//
//		for (unsigned i = 0; i < vals.size(); i++) {
//			int v = vals[i];
//			//For intermediate colname in root node, introduce a wild card.
//			if ((id == "0") && contains(int_str_map_[v], "MORPH")) {
//				expr card = ctx.int_const(int_str_map_[v].c_str());
//				s1 = Util::set_add(s1, card);
//			} else {
//				s1 = Util::set_add(s1, ctx.int_val(v));
//			}
//		}

		expr cst = ((rowCst == rowNum) && (colCst == colNum));
		if (!inputVec.empty()) {
			std::string headCstName = "HEAD_" + id;
			std::string contentCstName = "CONTENT_" + id;
//			std::cout << "Node content:" << contentCstName << std::endl;
			expr extra = getInOutConstraint(ctx, ds, inputVec, headCstName,
					contentCstName, (inputVec.size() > 1));
			cst = cst && extra;
		}

		z3::expr* e = new z3::expr(ctx, cst);
		return e;
	}

	static bool startsWith(std::string prefix, std::string toCheck) {
		return std::equal(prefix.begin(), prefix.end(), toCheck.begin());
	}

	static double timeDiff(int &t1, int &t2) {
		double diff = (t2 - t1) / double(CLOCKS_PER_SEC) * 1000;
		return diff;
	}

	//http://stackoverflow.com/questions/236129/split-a-string-in-c
	static std::vector<std::string> split(const std::string &text, char sep) {
		std::vector<std::string> tokens;
		std::size_t start = 0, end = 0;
		while ((end = text.find(sep, start)) != std::string::npos) {
			tokens.push_back(text.substr(start, end - start));
			start = end + 1;
		}
		tokens.push_back(text.substr(start));
		return tokens;
	}

	static int str2int(const std::string str) {
		char table[1024];
		strcpy(table, str.c_str());
		int val = atoi(table);
		return val;
	}

	static bool isTrueExpr(z3::expr interp) {
		z3::expr truthExpr = interp.ctx().bool_val(true);
		return Z3_is_eq_ast(truthExpr.ctx(), truthExpr, interp);
	}

	static inline bool is_number(const std::string& s) {
		return s.find_first_not_of(".,0123456789") == std::string::npos;
	}

	static std::string getColName() {
		std::string col = "MORPH" + std::to_string(counter_);
		counter_++;
		return col;
	}

	static int countSubStr(std::string org, std::string sub) {
		int count = 0;
		size_t nPos = org.find(sub, 0); // fist occurrence
		while (nPos != std::string::npos) {
			count++;
			nPos = org.find(sub, nPos + 1);
		}
		return count;
	}

	static std::string vec2string(std::vector<std::string> vec) {
		std::string str = "";

		for (unsigned i = 0; i < vec.size() - 1; i++) {

			assert(!vec[i].empty());
			if (vec[i][0] == '-') {
				std::string s(vec[i].begin() + 1, vec[i].end());
				str += "-`" + s + "`";
				str += ",";
			} else {
				str += "`" + vec[i] + "`";
				str += ",";
			}
		}

		assert(!vec.back().empty());

		if (vec.back()[0] == '-') {
			std::string s(vec.back().begin() + 1, vec.back().end());
			str += "-`" + s + "`";
		} else {
			str += "`" + vec.back() + "`";
		}
		return str;
	}

	static std::string vec2string2(std::vector<std::string> vec) {
		std::string str = "";

		for (unsigned i = 0; i < vec.size() - 1; i++) {

			assert(!vec[i].empty());
			if (vec[i][0] == '-') {
				std::string s(vec[i].begin() + 1, vec[i].end());
				str += "-\"" + s + "\"";
				str += ",";
			} else {
				str += "\"" + vec[i] + "\"";
				str += ",";
			}
		}

		assert(!vec.back().empty());

		if (vec.back()[0] == '-') {
			std::string s(vec.back().begin() + 1, vec.back().end());
			str += "-\"" + s + "\"";
		} else {
			str += "\"" + vec.back() + "\"";
		}
		return str;
	}

	static bool vecEqual(std::vector<int> v1, std::vector<int> v2) {
		if (v1.size() != v2.size())
			return false;
		else {
			for (unsigned i = 0; i < v1.size(); i++) {
				if (v1[i] != v2[i])
					return false;
			}
			return true;
		}
	}

	static std::string getFreshVar() {
		std::string varName = "var_" + std::to_string(var_counter_);
		var_counter_++;
		return varName;
	}

	//only use for debugging.
	static std::string expr2String(z3::expr e) {
		std::stringstream sstream;
		sstream << e;
		std::string s = sstream.str();
		return s;
	}

	static std::unordered_set<int> setDiff(std::unordered_set<int> s1,
			std::unordered_set<int> s2) {
		// Fill in s1 and s2 with values
//		std::set<int> result;
//		std::set_difference(s1.begin(), s1.end(), s2.begin(), s2.end(),
//				std::inserter(result, result.end()));

		std::unordered_set<int> result;
		for (std::unordered_set<int>::iterator it = s1.begin(); it != s1.end();
				it++) {
			if (s2.find(*it) == s2.end())
				result.insert(*it);
		}

		return result;
	}

	static z3::sort get_set_sort(context& ctx) {
		return ctx.int_sort();
	}

	static expr set_expr(context& ctx, std::string str) {
		z3::sort elt_type = get_set_sort(ctx);
		z3::sort set_sort = ctx.array_sort(elt_type, ctx.bool_sort());
		expr set_expr = ctx.constant(str.c_str(), set_sort);
		return set_expr;
	}

	//http://stackoverflow.com/questions/39608475/defining-a-theory-of-sets-with-z3-using-c
	static inline expr empty_set(sort const& s) {
		MK_EXPR1(Z3_mk_empty_set, s);
	}

	static inline expr full_set(sort const& s) {
		MK_EXPR1(Z3_mk_full_set, s);
	}

	static inline expr set_add(expr const& s, expr const& e) {
		MK_EXPR2(Z3_mk_set_add, s, e);
	}

	static inline expr set_del(expr const& s, expr const& e) {
		MK_EXPR2(Z3_mk_set_del, s, e);
	}

	static inline expr set_union(expr const& a, expr const& b) {
		check_context(a, b);
		Z3_ast es[2] = { a, b };
		Z3_ast r = Z3_mk_set_union(a.ctx(), 2, es);
		a.check_error();
		return expr(a.ctx(), r);
	}

	static inline expr set_intersect(expr const& a, expr const& b) {
		check_context(a, b);
		Z3_ast es[2] = { a, b };
		Z3_ast r = Z3_mk_set_intersect(a.ctx(), 2, es);
		a.check_error();
		return expr(a.ctx(), r);
	}

	static inline expr set_difference(expr const& a, expr const& b) {
		MK_EXPR2(Z3_mk_set_difference, a, b);
	}

	static inline expr set_complement(expr const& a) {
		MK_EXPR1(Z3_mk_set_complement, a);
	}

	static inline expr set_member(expr const& s, expr const& e) {
		MK_EXPR2(Z3_mk_set_member, s, e);
	}

	// a is subset of b
	static inline expr set_subset(expr const& a, expr const& b) {
		MK_EXPR2(Z3_mk_set_subset, a, b);
	}

	virtual ~Util();
}
;

#endif /* UTIL_UTIL_H_ */
