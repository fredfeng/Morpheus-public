#include <SketchFill.h>
#include <iterator>
#include <string>

std::vector<std::string> SketchFill::fill(Deduction *deduct,
		Benchmark *benchmark, RInside *R, Predicate *predicate,
		std::unordered_map<int, Component*> &specs,
		std::vector<Rcpp::DataFrame*>& dataframes) {

	unsigned table_cached_id = 0;

	Rcpp::DataFrame * df_output = new DataFrame(
			runRNT(R, benchmark->getOutput()));

	std::vector<std::string> code;

	std::deque<Node*> input_components;
	std::deque<Node*> working;

	Node *node = deduct->getTree()->getRoot();
	Node *recover = NULL;
	Node *split = NULL;
	Node *split_right_leaf = NULL;

	working.push_front(node);

	std::deque<Node*> working_bfs;
	working_bfs.push_front(node);

	std::deque<Node*> current_level;
	while (!working_bfs.empty()) {

		for (unsigned i = 0; i < working_bfs.size(); i++)
			current_level.push_back(working_bfs[i]);

		working_bfs.clear();

		while (!current_level.empty()) {
			Node * current_node = current_level.back();
			input_components.push_front(current_node);

			if (current_node->right != NULL && current_node->right->val != -1) {
				working_bfs.push_front(current_node->right);
			}

			if (current_node->left != NULL && current_node->left->val != -1) {
				working_bfs.push_front(current_node->left);
			}
			current_level.pop_back();
		}
	}
	assert(input_components.size() > 0);

	// create some IR for the input/ouput tables
	std::string str_goal = "morpheus";
	std::string str_output = Util::append("TBL_", input_components.back()->id);
	cmd_ = str_output + "=" + benchmark->getOutput();
	runRQNT(R, cmd_);
	std::vector<std::string> str_inputs;
	std::map<int, int> model2inputs;

	// remove input tables from the components and create intermediate tables
	std::vector<Node*> components;
	std::vector<bool> is_input_components(input_components.size(), false);

	for (unsigned int i = 0; i < input_components.size(); i++) {
		if (input_components[i]->val == 0) {
			std::string name = Util::append("TBL_", input_components[i]->id);
			str_inputs.push_back(name);
			assert(
					deduct->getNode2Input().find(input_components[i]->id)
							!= deduct->getNode2Input().end());
			int table_id = deduct->getNode2Input()[input_components[i]->id];
			cmd_ = name + "=" + benchmark->getInput()[table_id];

			model2inputs[str_inputs.size() - 1] = table_id;
			runRQNT(R, cmd_);
			is_input_components[i] = true;
		}
	}

	std::map<Node*, int> node2index;

	for (unsigned int i = 0; i < is_input_components.size(); i++) {
		if (!is_input_components[i]) {
			if (getInputTables(specs[input_components[i]->val]) == 2) {
				assert(split == NULL);
				split = input_components[i];
			}
			components.push_back(input_components[i]);
			node2index[input_components[i]] = components.size() - 1;
		}
	}

	// map id of data frame to
	std::map<int, int> dataframe2id;

	assert(components.size() > 0 && components[components.size() - 1]->id == 0);

	// create initial data frames
	int counter = 1;
	for (unsigned i = 0; i < components.size(); i++) {
		Rcpp::DataFrame *df = NULL;
		dataframes.push_back(df);

		if (components[i]->id == 0)
			dataframe2id[components[i]->id] = 0;
		else {
			dataframe2id[components[i]->id] = counter;
			counter++;
		}
	}
	// components[0] will always contain the output table

	State state = init;
	node = components.front();

	bool split_decision = false;
	bool split_backtrack = false;

	Node * rt = deduct->getTree()->getRoot();
	if (rt->val == 12 && rt->left != NULL && rt->right != NULL
			&& rt->left->val > 0 && rt->right->val > 0)
		split_decision = true;

//	for (unsigned i=0; i < components.size(); i++){
//		if (components[i]->val == 12)
//			split_decision = true;
//	}

	while (true) {

//		std::cout << "state=" << getState(state) << " | node=" << node->id
//				<< std::endl;

		if (state == end)
			break;

		switch (state) {

		case init:

//			std::cout << "before init preds size=" << node->preds.size()
//					<< std::endl;

			if (node->preds.size() > 0) {
				//state = decision;
				state = next;
			} else {

				if (node == split) {
					// no predicates are needed
					state = decision;

				} else if (node->left != NULL && node->left->val != 0) {

					if (split != NULL && node == split->left
							&& split_backtrack) {
						state = backtrack;
						split_backtrack = false;
					} else {

						// one children that is not input
						std::string name = Util::append("TBL_", node->left->id);
						assert(node->left->dataframe->nrows() > 0);
						predicate->generatePredicates(node->left->dataframe,
								node->preds, specs[node->val], name,
								gather_str_, node->left->group_char);

						state = decision;
					}

				} else if (node->left != NULL && node->left->val == 0) {

					if (split != NULL && node == split->left
							&& split_backtrack) {
						state = backtrack;
						split_backtrack = false;
					} else {

						// one children that is input
						int table_id = deduct->getNode2Input()[node->left->id];
						assert(node->left->dataframe->nrows() > 0);
						predicate->generatePredicates(node->left->dataframe,
								node->preds, specs[node->val],
								benchmark->getInput()[table_id], gather_str_,
								node->left->group_char);

						state = decision;
					}
				}
			}
//			std::cout << "predicates=" << node->preds.size() << std::endl;

			break;

		case decision:
			itn_limit_++;
			if (itn_limit_ > 20000) {
				state = unsat;
				break;
			}

			//std::cout << "predicates=" << node->preds.size() << std::endl;
			// no predicates were generated
			if (node->preds.empty()) {

				// there are no predicates for binary components
				if (node == split) {

					if (recover != NULL) {
						// go to the recover node before doing the split node
						assert(recover->val != 0 && recover->val != -1);
						state = next;
					} else {

						assert(
								node->left != NULL && node->left >= 0 && node->right != NULL && node->right >=0);
						std::string str_inner_output = Util::append("TBL_",
								node->id);

						std::vector<std::string> str_inner_inputs;
						if (node->left != NULL && node->left->val != -1) {
							str_inner_inputs.push_back(
									Util::append("TBL_", node->left->id));
						}
						if (node->right != NULL && node->right->val != -1) {
							str_inner_inputs.push_back(
									Util::append("TBL_", node->right->id));
						}

						assert(str_inner_inputs.size() > 0);
						//if (str_inner_inputs.size() > 1 && node->left->val != 0 && node->right->val != 0)
						if (str_inner_inputs.size() > 1 && node->left->val != 0
								&& node->right->val != 0)
							split_decision = true;

						std::vector<std::string> no_preds;

						std::string line = synthesizeLine(specs[node->val],
								str_inner_inputs, str_inner_output, no_preds);

						bool inner_error = false;
						bool cbind_error = false;

						// inner_join speed

						if (line.find("inner_join") != std::string::npos) {
							int t1 = -1;
							int t2 = -1;
							std::string number = "";
							for (unsigned i = 15; i < line.size(); i++) {
								if (line[i] == '_') {
									if (t1 == -1) {
										number = "";
										while (std::isdigit(line[i + 1])) {
											number += std::to_string(
													line[i + 1] - '0');
											i++;
										}
										t1 = std::stoi(number);
									} else {
										number = "";
										while (std::isdigit(line[i + 1])) {
											number += std::to_string(
													line[i + 1] - '0');
											i++;
										}
										t2 = std::stoi(number);
									}
								}
							}

							std::string s1 = "TBL_" + std::to_string(t1);
							std::string s2 = "TBL_" + std::to_string(t2);
							Rcpp::DataFrame * d1 = new DataFrame(runRNT(R, s1));
							Rcpp::DataFrame * d2 = new DataFrame(runRNT(R, s2));

							CharacterVector cv = d1->names();
							std::set<std::string> colnamesd1;
							for (unsigned i = 0; i < cv.size(); i++) {
								colnamesd1.insert(std::string(cv[i]));
							}

							CharacterVector cv2 = d2->names();
							std::set<std::string> colnamesd2;
							for (unsigned i = 0; i < cv2.size(); i++) {
								colnamesd2.insert(std::string(cv2[i]));
							}

							std::vector<std::string> set_intersect;
							set_intersection(colnamesd1.begin(),
									colnamesd1.end(), colnamesd2.begin(),
									colnamesd2.end(),
									std::back_inserter(set_intersect));

							inner_error = (set_intersect.size() > 2
									|| set_intersect.empty());
						}

						// cbind speedup to avoid errors -- should this be left to the deduction?
						if (line.find("cbind") != std::string::npos) {
							int t1 = -1;
							int t2 = -1;
							std::string number = "";
							for (unsigned i = 4; i < line.size(); i++) {
								if (line[i] == '_') {
									if (t1 == -1) {
										number = "";
										while (std::isdigit(line[i + 1])) {
											number += std::to_string(
													line[i + 1] - '0');
											i++;
										}
										t1 = std::stoi(number);
									} else {
										number = "";
										while (std::isdigit(line[i + 1])) {
											number += std::to_string(
													line[i + 1] - '0');
											i++;
										}
										t2 = std::stoi(number);
									}
								}
							}

							std::string s1 = "TBL_" + std::to_string(t1);
							std::string s2 = "TBL_" + std::to_string(t2);
							Rcpp::DataFrame * d1 = new DataFrame(runRNT(R, s1));
							Rcpp::DataFrame * d2 = new DataFrame(runRNT(R, s2));
//							Rcpp::DataFrame * d1 = new DataFrame(
//									(SEXP) R->parseEval(s1));
//							Rcpp::DataFrame * d2 = new DataFrame(
//									(SEXP) R->parseEval(s2));
							cbind_error = (d1->nrows() != d2->nrows());
						}

						std::string cache_str = "";
						bool cached = false;

						if (enable_cache_) {
							cache_str = convertRtoString(deduct, R, line);
							if (cache_code_.find(cache_str)
									!= cache_code_.end()) {
								stats_cached_++;
								cached = true;
								if (cache_code_[cache_str] == -1) {
									state = conflict;
								} else {
									state = deduction;
									node->code = line;
									runR(R,
											"TBL_" + std::to_string(node->id)
													+ "=TBL_CACHED_"
													+ std::to_string(
															cache_code_[cache_str]));
									deduct->getTree()->attachDataFrame2Node(
											node->id,
											cache_store_[cache_code_[cache_str]]);
								}
							}
						}

						if (!cbind_error && !inner_error && !cached) {

							Util::message(1, "Running code: %s", line.c_str());
							try {
								std::string cache_tbl = "";
								Rcpp::DataFrame * local;

								if (enable_cache_) {
									std::string cache_tbl = Util::append(
											"TBL_CACHED_", table_cached_id);
									local = new DataFrame(
											runR(R, cache_tbl + "=" + line));
								} else
									local = new DataFrame(runR(R, line));

								bool prune = pruneTables(R, str_inner_output,
										local);
								if (prune) {
									state = conflict;
									stats_pruned_++;
								} else {

									dataframes[dataframe2id[node->id]] = local;

									if (dataframes[dataframe2id[node->id]]->nrows()
											== 0)
										state = conflict;
									else {
										node->code = line;
										deduct->getTree()->attachDataFrame2Node(
												node->id,
												dataframes[dataframe2id[node->id]]);
										state = deduction;
										if (enable_cache_) {
											cache_code_[cache_str] =
													cache_store_.size();
											cache_store_.push_back(local);
											table_cached_id++;
										}
									}
								}

							} catch (const std::runtime_error& e) {
								state = conflict;
							}
						} else
							state = conflict;

					}
				} else
					state = conflict;

			} else {

				std::string str_inner_output = Util::append("TBL_", node->id);
				std::vector<std::string> str_inner_inputs;
				if (node->left != NULL && node->left->val != -1) {
					str_inner_inputs.push_back(
							Util::append("TBL_", node->left->id));
				}
				if (node->right != NULL && node->right->val != -1) {
					str_inner_inputs.push_back(
							Util::append("TBL_", node->right->id));
				}

				assert(str_inner_inputs.size() > 0);

				std::string line = synthesizeLine(specs[node->val],
						str_inner_inputs, str_inner_output, node->preds.back());
				node->preds.pop_back();

				std::string cache_str = "";
				bool cached = false;

				if (enable_cache_) {
					cache_str = convertRtoString(deduct, R, line);
					if (cache_code_.find(cache_str) != cache_code_.end()) {
						stats_cached_++;
						cached = true;
						if (cache_code_[cache_str] == -1) {
							state = conflict;
						} else {
							state = deduction;
							node->code = line;
							runR(R,
									"TBL_" + std::to_string(node->id)
											+ "=TBL_CACHED_"
											+ std::to_string(
													cache_code_[cache_str]));
							deduct->getTree()->attachDataFrame2Node(node->id,
									cache_store_[cache_code_[cache_str]]);
						}
					}
				}

				if (!cached) {
					Util::message(1, "Running code: %s", line.c_str());
					try {

						std::string cache_tbl = "";
						Rcpp::DataFrame * local;

						if (enable_cache_) {
							std::string cache_tbl = Util::append("TBL_CACHED_",
									table_cached_id);
							local = new DataFrame(
									runR(R, cache_tbl + "=" + line));
						} else
							local = new DataFrame(runR(R, line));

						bool prune = pruneTables(R, str_inner_output, local);

						if (prune) {
							if (enable_cache_)
								cache_code_[cache_str] = -1;
							state = conflict;
							stats_pruned_++;
						} else {
							dataframes[dataframe2id[node->id]] = local;

							if (dataframes[dataframe2id[node->id]]->nrows()
									== 0) {
								if (enable_cache_)
									cache_code_[cache_str] = -1;
								state = conflict;
								stats_pruned_++;
							} else {
								node->code = line;

//								if (line.find("group_by")!=std::string::npos){
//									//node->group_char = getCharGroups(node->id,R);
//									//node->group_num = getNumGroups(node->id,R);
//
//									std::size_t pos_begin = line.find("(");
//									std::size_t pos_end = line.find(")");
//									std::string groups = line.substr(pos_begin,pos_end);
//									char chars[] = "()`";
//									for (unsigned i = 0; i < strlen(chars); i++)
//										groups.erase(std::remove(groups.begin(), groups.end(), chars[i]), groups.end());
//									std::vector<std::string> g = Util::split(groups,',');
//									for (unsigned i = 1; i < g.size(); i++){
//										node->group_char.insert(g[i]);
//									}
//								}

								deduct->getTree()->attachDataFrame2Node(
										node->id,
										dataframes[dataframe2id[node->id]]);
								if (enable_cache_) {
									cache_code_[cache_str] =
											cache_store_.size();
									cache_store_.push_back(local);
									table_cached_id++;
								}
								state = deduction;
							}

						}

					} catch (const std::runtime_error& e) {
						if (enable_cache_)
							cache_code_[cache_str] = -1;
						state = conflict;
					}

				}
			}

			break;

		case deduction:

			// ask the unifier if the current system is still unifiable
			if (isUnifiable(deduct, partialevaluation_)) {
				// is this the root node?
				if (node == deduct->getTree()->getRoot()) {
					state = test;
				} else {
					state = next;
				}
				stats_predicate_accepted_++;
			} else {
				Util::message(1,
						"Deduction rejected the predicate; nrows=%d\tncols:%d",
						node->dataframe->nrows(), node->dataframe->size());
				stats_predicate_rejected_++;
				state = conflict;
			}
			break;

		case next:
			//std::cout << "recover is null=" << (recover == NULL) << std::endl;

			if (recover != NULL) {
				if (node->preds.empty())
					split_backtrack = true;

				node = recover;
				assert(recover->val != 0 && recover->val != -1);
				assert(
						recover->left != NULL && recover->right != NULL && recover->right->val == -1 && (recover->left->val == 0 || recover->left->val == -1));
				recover = NULL;

			} else {
				unsigned int next_index = node2index[node] + 1;
				assert(next_index < components.size());
				node = components[next_index];
			}
			state = init;
			break;

		case test:
			assert(node == deduct->getTree()->getRoot());
			getCode(node, code);

			if (passTest(code, node->dataframe, benchmark, R, dataframes,
					df_output)) {
				state = sat;
			} else {
				code.clear();
				state = conflict;
			}

			break;

		case conflict:

			// there are still predicates in the current node
			if (!node->preds.empty()) {
				state = decision;

			} else {
				// it is not possible to backtrack anymore
				if (node->left != NULL && node->left->val != 0
						&& node->left->val != -1) {
					state = backtrack;
				} else {
					if (node == split) {
						state = backtrack;
					} else if (split_decision && split_right_leaf == NULL) {
						split_right_leaf = node;
						state = backtrack;
					} else if (split_right_leaf != NULL
							&& node == split_right_leaf) {
						state = backtrack;
					} else
						state = unsat;
				}
			}
			break;

		case backtrack:

			// for chains
			deduct->getTree()->delDataFrame(node);
			node->code = "";
			node->group_char.clear();
			node->group_num = 0;

//			id_current = node->id;

			if (node->val == 2) {
				if (!gather_str_.empty()) {
					assert(gather_str_.size() == 2 || gather_str_.size() == 4);
					gather_str_.pop_back();
					gather_str_.pop_back();
				}
			}

			if (node == split_right_leaf) {

				assert(split->left != NULL);

				if (split->left->val == 0) {
					// input on the left child
					state = unsat;
				} else {

					assert(
							split->left != NULL && split->left->val != 0 && split->left->val != -1);
//					std::cout << "recover node is now =" << node->id
//							<< std::endl;
					recover = node;
					node = split->left;
					state = decision;
				}

			} else if (node->left == split || node == split) {

				if (split->right->val == 0) {
					if (split->left->val == 0) {
						// both left and right are input tables
						state = unsat;
					} else {
						// right child is input table
						node = split->left;
						state = decision;
						deduct->getTree()->delDataFrame(split);
						split->code = "";
					}
				} else {
					// has two children
					node = split->right;
					state = decision;
					deduct->getTree()->delDataFrame(split);
					split->code = "";
				}

			} else {

				if (node->left != NULL
						&& (node->left->val == -1 || node->left->val == 0)) {
					state = unsat;

				} else {
					assert(
							node->left != NULL && node->left->val != 0 && node->left->val != -1);

					node = node->left;
					state = decision;
				}
			}

//			id_backtrack = node->id;
//			if (id_current == 2 && id_backtrack == 1)
//				assert(false);

			break;

		case sat:
			//getCode(deduct->getTree()->getRoot(), code);

			//renameDataFrame(benchmark->getOutput(), str_output, code, R);

			for (unsigned i = 0; i < code.size(); i++) {
				for (unsigned j = 0; j < str_inputs.size(); j++)
					Util::replaceAll(code[i], str_inputs[j],
							benchmark->getInput()[model2inputs[j]]);
				Util::replaceAll(code[i], str_output, str_goal);
			}

			//code.push_back(str_goal + "=as.data.frame(" + str_goal + ")");
			code.push_back(str_goal);

			state = end;
			break;

		case unsat:
			code.clear();
			state = end;
			break;

		case end:
			assert(false);
			break;

		}

	}

	delete df_output;
	return code;
}

void SketchFill::getCode(Node * root, std::vector<std::string>& code) {

	code.clear();
	Node* node = root;

	std::deque<std::string> result;
	std::deque<Node*> working;

	working.push_front(node);

	while (!working.empty()) {

		if (node->val != 0)
			result.push_front(node->code);

		if (node->left != NULL && node->left->val != -1)
			working.push_back(node->left);

		if (node->right != NULL && node->right->val != -1)
			working.push_back(node->right);

		working.pop_front();
		if (!working.empty())
			node = working.front();
	}

	for (unsigned i = 0; i < result.size(); i++) {
		code.push_back(result[i]);
	}
}

std::string SketchFill::synthesizeLine(Component * comp,
		std::vector<std::string>& inputs, std::string output,
		std::vector<std::string>& predicate) {

	std::vector<std::string> gather_strings;
	if (!comp->getName().compare("gather")) {
		assert(predicate.size() > 2);
		gather_strings.push_back(predicate[predicate.size() - 1]);
		gather_strings.push_back(predicate[predicate.size() - 2]);
		predicate.pop_back();
		predicate.pop_back();
	}

	std::string str_template = comp->getTemplate();
// replace input table
	unsigned tblCount = Util::countSubStr(str_template, "#Table");
	for (unsigned i = 0; i < tblCount; i++) {
		std::string to_rep = "#Table" + std::to_string(i);
		Util::replaceAll(str_template, to_rep, inputs[i]);
	}

// introduce fresh name for constant column names
	unsigned colCount = Util::countSubStr(str_template, "#STR");
	for (unsigned i = 0; i < colCount; i++) {
		std::string to_rep = "#STR" + std::to_string(i);
		if (!gather_strings.empty()) {
			assert(colCount == gather_strings.size());
			Util::replaceAll(str_template, to_rep, gather_strings[i]);
		} else
			Util::replaceAll(str_template, to_rep, Util::getColName());
	}
//if it's unbounded predicates, i.e., column names
	if (str_template.find("#Unknown") != std::string::npos) {
		std::string preds = Util::vec2string(predicate);
		Util::replaceAll(str_template, "#Unknown", preds);
		//normal predicates
	} else {
		// replace holes
		for (unsigned i = 0; i < predicate.size(); i++) {
			std::string to_rep = "#" + std::to_string(i);
			if (!predicate[i].compare(""))
				to_rep = "`#" + std::to_string(i) + "`";
			Util::replaceAll(str_template, to_rep, predicate[i]);
		}
	}

	std::string line = output + "=" + str_template;

	return line;
}

bool SketchFill::passTest(std::vector<std::string>& code, Rcpp::DataFrame* df,
		Benchmark *benchmark, RInside *R,
		std::vector<Rcpp::DataFrame*>& dataframes, Rcpp::DataFrame *df_out) {

	assert(df_out != NULL && dataframes[0] != NULL);

	std::string output = benchmark->getOutput();
	Rcpp::DataFrame* expected_output = df_out;
	Rcpp::DataFrame* current_output = dataframes[0];

// if the deduction system is used then this cannot return false!
	if (expected_output->nrows() != current_output->nrows()
			|| expected_output->size() != current_output->size()) {
		return false;
	}

	std::string convert2df = "TBL_0=as.data.frame(TBL_0)";
	code.push_back(convert2df);
	dataframes[0] = new DataFrame(runRNT(R, convert2df));
	df = dataframes[0];

//	for (unsigned i = 0; i < code.size(); i++)
//		std::cout << "code=" << code[i] << std::endl;

// check if a char column should be converted into numeric
// FIXME: should this check be done during the execution of the algorithm?
	std::vector<std::string> coltypes = Util::getColTypes(df, "TBL_0");
	std::vector<std::string> coltypes_original = Util::getColTypes(df_out,
			output);
	std::vector<std::vector<std::string>> matrix = Util::convertToMatrix(df);

//	Util::printDataFrame(df);
//	Util::fastConvert(df);

	bool coerce = false;

	for (unsigned i = 0; i < coltypes.size(); i++) {
		if (!coltypes[i].compare("character")) {
			bool number = true;
			for (unsigned j = 0; j < matrix.size(); j++) {
				if (!Util::is_number(matrix[j][i])) {
					number = false;
				}
			}
			if (number) {
				// convert column to numeric type
				std::string convert_code = "TBL_0[," + std::to_string(i + 1)
						+ "] = as.numeric(TBL_0[," + std::to_string(i + 1)
						+ "])";
				//std::cout << "code=" << convert_code << std::endl;
				code.push_back(convert_code);
				runRQNT(R, convert_code);
				coltypes[i] = "numeric";
				coerce = true;
			}
		}
	}

	bool res = runRNTBoolean(R,
			"isTRUE(compare(TBL_0," + output
					+ ", round=function(x) { signif(x, 3) }))");

	std::set<std::string> output_char;
	std::set<double> output_double;

	if (res){
		res=renameDataFrame(output,"TBL_0",code,R);
	}

	if (!res) {
		unsigned int n_mapped = 0;

		std::vector<std::multiset<double>> output_double;
		std::vector<std::multiset<std::string>> output_char;

		//Util::printDataFrame(current_output);

		if (!coerce) {
			output_double = Util::getNumColValues(current_output);
			output_char = Util::getCharColValues(current_output);
		} else {
			Rcpp::DataFrame cout = runR(R, "TBL_0");
			//Util::printDataFrame(&cout);
			output_double = Util::getNumColValues(&cout);
			output_char = Util::getCharColValues(&cout);
		}

		std::vector<int> mapped_double(output_double.size(), -1);
		std::vector<int> mapped_char(output_char.size(), -1);
		std::vector<int> mapped(coltypes.size(), -1);
		std::map<int, int> char2all;

		int char_n = 0;
		for (unsigned i = 0; i < coltypes.size(); i++) {
			if (!coltypes[i].compare("character")) {
				char2all[char_n] = i;
				char_n++;
			}
		}

		if (output_double.size() == output_double_.size()
				&& output_char.size() == output_char_.size()) {

			for (unsigned i = 0; i < coltypes.size(); i++) {
				if (!coltypes[i].compare("character"))
					continue;

				for (unsigned j = 0; j < coltypes.size(); j++) {

					if (!coltypes[i].compare("numeric")
							&& !coltypes_original[j].compare("numeric")) {

						if (mapped[j] != -1)
							continue;

						std::string equal_code =
								"isTRUE(compare(TBL_0[," + std::to_string(i + 1)
										+ "]," + output + "[,"
										+ std::to_string(j + 1)
										+ "],ignoreOrder=TRUE, round=function(x) { signif(x, 3) }))";
						bool equal = runRNTBoolean(R, equal_code);
						if (equal) {
							mapped[j] = i;
							n_mapped++;
							break;
						}
					}
				}
			}

			for (unsigned i = 0; i < output_char.size(); i++) {
				for (unsigned j = 0; j < output_char.size(); j++) {
					if (mapped_char[j] != -1)
						continue;

					std::multiset<std::string> result;
					std::set_difference(output_char[i].begin(),
							output_char[i].end(), output_char_[i].begin(),
							output_char_[i].end(),
							std::inserter(result, result.begin()));
					if (result.empty()) {
						mapped_char[j] = i;
						n_mapped++;
						break;
					} else {
						// check if the difference is because of MORPH values
						unsigned morph = 0;
						for (std::multiset<std::string>::iterator it =
								result.begin(); it != result.end(); ++it) {
							//std::cout << "it=" << (*it) << std::endl;
							if ((*it).find("MORPH") != std::string::npos) {
								morph++;
							}
						}
						// FIXME: MORPH is being used as wildcard! this may lead to problems!
						if (morph == result.size()) {
							mapped_char[j] = i;
							n_mapped++;
							break;
						}
					}
				}
			}
		}

		if (n_mapped == coltypes.size()) {

			// replace the char mapping
			if (!mapped_char.empty()) {
				int ic = 0;
				for (unsigned i = 0; i < mapped.size(); i++) {
					if (mapped[i] == -1) {
						mapped[i] = char2all[ic];
						ic++;
					}
				}
				assert(ic == mapped_char.size());
			}

			bool sorted = true;
			for (unsigned i = 0; i < mapped.size(); i++) {
				if (mapped[i] != i) {
					sorted = false;
					break;
				}
			}

			// check if arrange is needed

			Rcpp::CharacterVector cv_out = current_output->names();
			std::vector<std::string> colnames_out(cv_out.size());
			for (unsigned i = 0; i < cv_out.size(); i++) {
				colnames_out[i] = std::string(cv_out[i]);
			}

			bool arrange_needed = false;
			std::vector<std::string> arrange_numeric;
			std::vector<int> arrange_index;

			for (unsigned i = 0; i < coltypes.size(); i++) {

				if (!coltypes[i].compare("character"))
					continue;

				arrange_numeric.push_back("`" + colnames_out[i] + "`");
				arrange_index.push_back(i);

				std::string equal_code = "isTRUE(compare(TBL_0[,"
						+ std::to_string(i + 1) + "]," + output + "[,"
						+ std::to_string(i + 1)
						+ "], round=function(x) { signif(x, 3) }))";
				bool equal = runRNTBoolean(R, equal_code);
				if (!equal) {
					arrange_needed = true;
				}
			}

			std::vector<std::vector<std::string>> comb;
			std::set<std::vector<std::string>> perm;
			unsigned n = arrange_numeric.size();
			for (unsigned r = 1; r <= n; r++) {

				std::vector<bool> v(n);
				std::fill(v.begin(), v.begin() + r, true);

				do {
					std::vector<std::string> pred;
					for (unsigned i = 0; i < n; ++i) {
						if (v[i]) {
							pred.push_back(arrange_numeric[i]);
						}
					}

					if (pred.size() <= 2)
						comb.push_back(pred);

				} while (std::prev_permutation(v.begin(), v.end()));
			}

			for (unsigned j = 0; j < comb.size(); j++) {

				unsigned n = comb[j].size();
				do {
					std::vector<std::string> pred;
					for (unsigned i = 0; i < n; ++i) {
						pred.push_back(comb[j][i]);
					}

					if (pred.size() == 1) {
						std::vector<std::string> pred_desc;
						pred_desc.push_back("desc(" + pred[0] + ")");
						perm.insert(pred_desc);
					} else if (pred.size() == 2) {
						std::vector<std::string> pred_desc1;
						pred_desc1.push_back("desc(" + pred[0] + ")");
						pred_desc1.push_back("desc(" + pred[1] + ")");
						perm.insert(pred_desc1);

						std::vector<std::string> pred_desc2;
						pred_desc2.push_back(pred[0]);
						pred_desc2.push_back("desc(" + pred[1] + ")");
						perm.insert(pred_desc2);

						std::vector<std::string> pred_desc3;
						pred_desc3.push_back("desc(" + pred[0] + ")");
						pred_desc3.push_back(pred[1]);
						perm.insert(pred_desc3);
					}

					perm.insert(pred);

				} while (std::prev_permutation(comb[j].begin(), comb[j].end()));
			}

			if (arrange_needed) {
				for (std::set<std::vector<std::string>>::iterator it =
						perm.begin(); it != perm.end(); it++) {
					// check if there is a permutation of arrange that satisfied the benchmark
					std::string exec = "MORPH_ARR=arrange(TBL_0,";
					for (unsigned i = 0; i < (*it).size(); i++) {
						exec += (*it)[i];
						if (i < (*it).size() - 1)
							exec += ",";
						else
							exec += ")";
					}
					//std::cout << "exec=" << exec << std::endl;
					runRQNT(R, exec);

					// check that all columns match
					bool match = true;
					for (unsigned i = 0; i < arrange_index.size(); i++) {
						std::string equal_code = "isTRUE(compare(MORPH_ARR[,"
								+ std::to_string(arrange_index[i] + 1) + "],"
								+ output + "[,"
								+ std::to_string(arrange_index[i] + 1)
								+ "], round=function(x) { signif(x, 3) }))";
						bool equal = runRNTBoolean(R, equal_code);
						if (!equal) {
							match = false;
							break;
						}
					}
					if (match) {
						Util::replace(exec, "MORPH_ARR", "TBL_0");
						code.push_back(exec);
						runRQNT(R,exec);
						break;
					}
				}
			}

			if (!sorted) {

				Rcpp::CharacterVector cv = expected_output->names();
				std::vector<std::string> colnames(cv.size());
				for (unsigned i = 0; i < cv.size(); i++) {
					colnames[i] = std::string(cv[i]);
				}

				std::string as = "MORPH_SEL=select(TBL_0,";
				std::string s = "TBL_0=select(TBL_0,";
				for (unsigned i = 0; i < mapped.size(); i++) {
					as += std::to_string(mapped[i] + 1);
					s += std::to_string(mapped[i] + 1);
					if (i != mapped.size() - 1){
						s += ",";
						as += ",";
					} else {
						s += ")";
						as += ")";
					}
				}
				code.push_back(s);

				runRQNT(R, as);

				// is arranged still needed?
				arrange_needed = false;
				for (unsigned i = 0; i < coltypes.size(); i++) {
					if (!coltypes[mapped[i]].compare("character"))
						continue;

					std::string equal_code = "isTRUE(compare(MORPH_SEL[,"
							+ std::to_string(mapped[i] + 1) + "]," + output
							+ "[," + std::to_string(mapped[i] + 1)
							+ "], round=function(x) { signif(x, 3) }))";
					bool equal = runRNTBoolean(R, equal_code);
					if (!equal) {
						arrange_needed = true;
						break;
					}
				}

				if (arrange_needed) {
					for (std::set<std::vector<std::string>>::iterator it =
							perm.begin(); it != perm.end(); it++) {
						// check if there is a permutation of arrange that satisfied the benchmark
						std::string exec = "MORPH_ARR=arrange(MORPH_SEL,";
						for (unsigned i = 0; i < (*it).size(); i++) {
							exec += (*it)[i];
							if (i < (*it).size() - 1)
								exec += ",";
							else
								exec += ")";
						}
						runRQNT(R, exec);

						// check that all columns match
						bool match = true;
						for (unsigned i = 0; i < arrange_index.size(); i++) {
							std::string equal_code =
									"isTRUE(compare(MORPH_ARR[,"
											+ std::to_string(
													mapped[arrange_index[i]]
															+ 1) + "]," + output
											+ "[,"
											+ std::to_string(
													mapped[arrange_index[i]] + 1)
											+ "], round=function(x) { signif(x, 3) }))";
							bool equal = runRNTBoolean(R, equal_code);
							if (!equal) {
								match = false;
								break;
							}
						}
						if (match) {
							Util::replace(exec, "MORPH_ARR", "TBL_0");
							Util::replace(exec, "MORPH_SEL", "TBL_0");
							code.push_back(exec);
							runRQNT(R,exec);
							break;
						}
					}
				}

				//update table
				//Rcpp::DataFrame df_swp = runR(R, s);
				runRQNT(R, s);
				res = true;
				res = renameDataFrame(output,"TBL_0",code,R);

//				std::vector<std::string> rename;
//
//				Rcpp::CharacterVector cv2 = df_swp.names();
//				for (unsigned i = 0; i < cv2.size(); i++) {
//					std::string ss = std::string(cv2[i]);
//					if (colnames[i].compare(ss) != 0) {
//						if (ss.find("MORPH") != std::string::npos) {
//							std::string r_cmd = colnames[i] + "=" + ss;
//							rename.push_back(r_cmd);
//						} else
//							res = false;
//					}
//				}
//
//				if (res && !rename.empty()) {
//					std::string s = "TBL_0=rename(TBL_0,";
//					for (unsigned i = 0; i < rename.size(); i++) {
//						s += rename[i];
//						if (i != rename.size() - 1)
//							s += ",";
//					}
//					s += ")";
//					code.push_back(s);
//				}

			} else {
				res = true;
				res = renameDataFrame(output,"TBL_0",code,R);
			}
		}
		// FIXME: add the extra code for reordering rows!
	}

	return res;
}

bool SketchFill::renameDataFrame(const std::string &output,
		const std::string &synthesize, std::vector<std::string> &code,
		RInside *R) {

	Rcpp::DataFrame * out = new DataFrame((SEXP) R->parseEval(output));
	Rcpp::DataFrame * synth = new DataFrame((SEXP) R->parseEval(synthesize));

//	Util::printDataFrame(out);
//	Util::printDataFrame(synth);

	Rcpp::CharacterVector cout = out->names();
	Rcpp::CharacterVector csynth = synth->names();

	assert(cout.size() == csynth.size());

	std::map<std::string,std::string> rename_map;
	bool equal = true;

	for (int i = 0; i < cout.size(); i++) {
		if (std::string(cout[i]).compare(std::string(csynth[i]))) {
			rename_map[std::string(csynth[i])] = std::string(cout[i]);
			if (std::string(csynth[i]).find("MORPH")==std::string::npos){
				equal = false;
				break;
			}
//			std::string rename = "colnames(" + synthesize + ")["
//					+ std::to_string(i + 1) + "]=\"" + std::string(cout[i])
//					+ "\"";
//			code.push_back(rename);
		}
	}

	if (equal) {
		for (unsigned i = 0; i < code.size(); i++) {
			for (std::map<std::string, std::string>::iterator it =
					rename_map.begin(); it != rename_map.end(); ++it) {
				Util::replaceAll(code[i], it->first, it->second);
			}
		}
	}


	delete out;
	delete synth;

	return equal;
}

int SketchFill::getInputTables(Component *comp) {

	int tables = 0;
	for (const auto &type : comp->getArgTypes()) {
		if (!type.compare("Table")) {
			tables++;
		}
	}
	return tables;
}

bool SketchFill::pruneTables(RInside * R, std::string name,
		Rcpp::DataFrame *df) {

	bool too_large = false;
	bool has_na = R->parseEval("anyNA(" + name + ")");
	if (df->nrows() > 40 || df->size() > 20)
		too_large = true;

	return (has_na || too_large);

}

std::string SketchFill::convertRtoString(Deduction * deduct, RInside *R,
		std::string cmd) {

	std::string result = cmd;
	std::string table = "TBL_";

	std::vector<size_t> positions;
	std::set<std::string> table_ids;

	size_t pos = cmd.find(table, 0);
	while (pos != std::string::npos) {
		positions.push_back(pos);
		pos = cmd.find(table, pos + 1);
	}

	size_t current_pos = 0;
	for (unsigned i = 1; i < positions.size(); i++) {
		//std::cout << "position=" << positions[i] << std::endl;
		current_pos = positions[i] + 4;
		std::string number = "";
		while (std::isdigit(cmd[current_pos])) {
			number += cmd[current_pos];
			current_pos++;
		}
		//std::cout << "number=" << number << std::endl;
		table_ids.insert(number);
	}
	assert(!table_ids.empty());

	for (std::set<std::string>::iterator it = table_ids.begin();
			it != table_ids.end(); ++it) {
		std::string tbl_cmd = "TBL_" + *it;
		//Rcpp::DataFrame df = runRNT(R, tbl_cmd);
		assert(deduct->getTree()->getDataFrameById(std::stoi(*it))!=NULL);
		std::string tbl_output = Util::dataFrameToString(
				deduct->getTree()->getDataFrameById(std::stoi(*it)));
		Util::replaceAll(result, tbl_cmd, tbl_output);
	}

	return result;
}
