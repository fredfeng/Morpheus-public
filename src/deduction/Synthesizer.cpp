/*
 * Synthesizer.cpp
 *
 *  Created on: Aug 26, 2016
 *      Author: Ruben, Yu
 */

#include <Synthesizer.h>
#include <vector>
#include <string>
#include <iostream>
#include <RInside.h>                    // for the embedded R via RInside
#include <glob.h>
#include <tuple>
#include <deque>
#include "Util.h"
#include <ctime>// include this header

#include <SketchFill.h>

double deductTime = 0.0;

Synthesizer::Synthesizer(Benchmark* b, const char* spec_loc, int min, int max,
		bool cacher, bool forceskt, bool pe) :
		stats_predicate_rejected_(0), stats_predicate_accepted_(0), stats_pruned_(
				0), stats_R_(0), stats_R_time_(0), stats_Z3_pe_time_(0), stats_cached_R_(
				0) {
	// init benchmark.
	benchmark_ = b;
	benchmark_->dump();

	forceSketch_ = forceskt;
	cache_R_ = cacher;
	partialevaluation_ = pe;

	writeSketch_ = false;
	sketchFilename_ = "";

	numeric_comp_ = false;

	ngram_ = false;

	// Init R engine.
	// Load libraries and database.
	int t1 = clock();
	R_.parseEvalQNT(
			"library(MorpheusData) ; library(tidyr) ; library(dplyr) ; library(compare)");
	if(benchmark_->hasCsv()) R_.parseEvalQNT(benchmark_->getCsvStr());
	int t2 = clock();
	stats_R_ += 4;
	stats_R_time_ += Util::timeDiff(t1, t2);
//	R_.parseEvalQ("library(pryr)");
//	R_.parseEvalQ("options(show.error.messages = FALSE, warn = -1)");

	// init specs.
	std::cout << "Init specifications: " << spec_loc << "\n";

	glob_t glob_result;
	glob(spec_loc, GLOB_TILDE, NULL, &glob_result);
	for (unsigned int i = 0; i < glob_result.gl_pathc; ++i) {
		char* spec = glob_result.gl_pathv[i];
		Component* comp = new Component(spec);
		spec_map.insert(std::make_pair(comp->getId(), comp));
		std::cout << spec << " compName: " << comp->getName() << std::endl;
	}

	this->deduct_ = Deduction(spec_map);

	min_length_ = min;
	max_length_ = max;
}

std::vector<std::string> Synthesizer::synthesis() {
	std::cout << "begin to synthesize....." << std::endl;
	int time_begin = clock();

	int max_length = max_length_;
	int current_length = min_length_;

	if (writeSketch_)
		sketchFile_.open(sketchFilename_);

	SketchEncoding * sketch = new SketchEncoding(spec_map, current_length,
			ngram_);
	int nb_sketches = 0;
	int nb_all_sketches = 0;
	inputFrames_ = getInputDataFrames();
	deduct_.setInputTables(inputFrames_);
	Rcpp::DataFrame output_frame = getOutputFrame();
	output_double_ = Util::getNumColValues(&output_frame);
	output_char_ = Util::getCharColValues(&output_frame);

	// check if the numeric values in the output are the same as in the input
	std::set<double> num_input;
	std::set<double> num_output;

	for (unsigned i = 0; i < inputFrames_.size(); i++) {
		std::vector<std::multiset<double>> col = Util::getNumColValues(
				&inputFrames_[i]);
		for (unsigned j = 0; j < col.size(); j++) {
			for (std::multiset<double>::iterator it = col[j].begin();
					it != col[j].end(); it++) {
				num_input.insert(*it);
			}
		}
	}

	std::vector<std::multiset<double>> col = Util::getNumColValues(
			&output_frame);
	for (unsigned j = 0; j < col.size(); j++) {
		for (std::multiset<double>::iterator it = col[j].begin();
				it != col[j].end(); it++) {
			num_output.insert(*it);
		}
	}

	std::set<double> num_result;
	for (std::set<double>::iterator it = num_output.begin(); it != num_output.end(); ++it){
		if (num_input.find(*it)==num_input.end()){
			numeric_comp_ = true;
			break;
		}
	}

	if (!num_result.empty())
		numeric_comp_ = true;

	if (numeric_comp_)
		std::cout << "Numeric computations needed to solve this benchmark!"
				<< std::endl;

	/////////////////////////////////////////////////////////////////

	bool flag = true;
	std::vector<std::string> code;
	std::vector<std::string> inputNames;
	for (unsigned i = 0; i < benchmark_->getInput().size(); i++)
		inputNames.push_back(benchmark_->getInput()[i]);

	predicate_.generateInputConstant(inputFrames_, inputNames);

	while (current_length <= max_length) {

		Util::message(1,
				"=================================================================\nsketch size=%d\nsketch w/ deduction=%d\t\t sketch w/o deduction=%d\npredicates w/ deduction=%d\t predicates w/o deduction=%d",
				current_length, nb_sketches, nb_all_sketches,
				stats_predicate_accepted_,
				stats_predicate_accepted_ + stats_predicate_rejected_);
		std::vector<int> skt = generateSketch(sketch);
		//std::cout << "current sketch: " << this->sketch2String(skt) << std::endl;

//		if (forceSketch_)
//			skt = benchmark_->getSketch();

		if (skt.empty()) {
			// There are no more sketches for the current length
			current_length++;
			delete sketch;
			sketch = new SketchEncoding(spec_map, current_length, ngram_);
		} else {
			nb_all_sketches++;
			/////////////Inputs and output constraints.///////////////////
			int t1_deduct = clock();

			deduct_.buildSystem(skt, benchmark_, &R_);
			deduct_.setCacheR(cache_R_);

			if (writeSketch_)
				flag = true;
			else {
				flag = deduct_.solve();
			}

			int t2_deduct = clock();
			deductTime += Util::timeDiff(t1_deduct, t2_deduct);
			if (!flag) {
				Util::message(1, "Deduction rejected the sketch: %s",
						this->sketch2String(skt).c_str());
				deduct_.clean();
				continue;
			}

			nb_sketches++;
			bool done = false;
			int align_iterations = 0;
			while (deduct_.alignInputTables()) {
				align_iterations++;
				if (writeSketch_) {
					std::string skt_str = sketch2String(skt);
					if (skt_str.find("inner_join") != std::string::npos) {
						Node * node = deduct_.getTree()->getRoot();
						std::deque<std::string> p1;
						std::deque<std::string> p2;
						std::deque<std::string> p3;
						Node * node_p2 = NULL;
						Node * node_p3 = NULL;
						while (node != NULL && node->val != -1 && node->val != 0) {
							//inner_join
							if (node->val == 12) {
								node_p2 = node->left;
								node_p3 = node->right;
								break;
							} else {
								Component* comp = spec_map[node->val];
								std::string comp_str = comp->getName();
								p1.push_front(comp_str);
								node = node->left;
							}
						}

						node = node_p2;
						while (node != NULL && node->val != -1 && node->val != 0) {
							Component* comp = spec_map[node->val];
							std::string comp_str = comp->getName();
							p2.push_front(comp_str);
							node = node->left;
						}

						node = node_p3;
						while (node != NULL && node->val != -1 && node->val != 0) {
							Component* comp = spec_map[node->val];
							std::string comp_str = comp->getName();
							p3.push_front(comp_str);
							node = node->left;
						}

						std::string skt_new = "";
						for (unsigned i = 0; i < p2.size(); i++) {
							skt_new += p2[i];
							skt_new += " ";
						}

						for (unsigned i = 0; i < p3.size(); i++) {
							skt_new += p3[i];
							skt_new += " ";
						}

						skt_new += "inner_join ";
						for (unsigned i = 0; i < p1.size(); i++) {
							skt_new += p1[i];
							skt_new += " ";
						}

						sketchFile_ << skt_new;
//						sketchFile_ << "\n";
						sketchFile_ << "v= ";
						for (unsigned i = 0; i < skt.size(); i++) {
							sketchFile_ << skt[i] << " ";
						}
						sketchFile_ << "M\n";
						break;

					} else {
						sketchFile_ << skt_str;
						sketchFile_ << " v= ";
						for (unsigned i = 0; i < skt.size(); i++) {
							sketchFile_ << skt[i] << " ";
						}
						sketchFile_ << "M\n";
//						sketchFile_ << "\n";
						break;
					}
				}

				if (forceSketch_) {	//Enumerating sketches only.
					//std::cout << "current sketch: " << this->sketch2String(skt) << std::endl;
					if (Util::vecEqual(skt, benchmark_->getSketch())) {
						std::cout << "Correct sketch: " << sketch2String(skt)
								<< std::endl;
						code = fillSketch(cache_R_, partialevaluation_);
						if (code.empty()) {
							deduct_.blockSolution();
							if (deduct_.is_sat()) {
								std::cout << "Error: Get next model!"
										<< std::endl;
								continue;
							} else {
								std::cout << "Error: no solution found!"
										<< std::endl;
								exit(0);
							}
						}

//						assert(!code.empty());
						done = true;
						break;
					} else {
//						std::cout << "bad sketch: " << this->sketch2String(skt)
//								<< std::endl;
						deduct_.blockSolution();
						continue;
					}
				}

				code = fillSketch(cache_R_, partialevaluation_);

//				std::cout << "trying sketch: " << this->sketch2String(skt) << " Running time:" << Util::timeDiff(t2,t1) << std::endl;

				if (!code.empty()) {
					done = true;
					break;
				}
				deduct_.blockSolution();
			}
			deduct_.clean();
			assert(align_iterations > 0);
			if (done)
				break;
		}
	}

	int time_end = clock();
	double total_time = Util::timeDiff(time_begin, time_end);

	std::cout << "# sketches with deduction: " << nb_sketches << std::endl;
	std::cout << "# sketches without deduction: " << nb_all_sketches
			<< std::endl;

	std::cout << "# predicates with deduction: " << stats_predicate_accepted_
			<< std::endl;
	std::cout << "# predicates without deduction: "
			<< stats_predicate_accepted_ + stats_predicate_rejected_
			<< std::endl;

	//std::cout << "# NA tables: " << stats_na_ << std::endl;

	//std::cout << "# R tests: " << stats_r_tests_ << std::endl;

	std::cout << "# predicates pruned (NA or size): " << stats_pruned_
			<< std::endl;

	std::cout << "# R calls: " << stats_R_ << std::endl;

	std::cout << "# R cached calls: " << stats_cached_R_ << std::endl;

	std::cout << "==================Profiling (ms)=================="
			<< std::endl;
	std::cout << "Deduction on sketch generation: " << deductTime << std::endl;
	std::cout << "Deduction on partial evaluation:" << stats_Z3_pe_time_
			<< std::endl;
	std::cout << "R calls: " << stats_R_time_ << std::endl;
	std::cout << "Total time: " << total_time << std::endl;
	std::cout << "[Percentage] R=" << stats_R_time_ / total_time * 100 << " Z3="
			<< (deductTime + stats_Z3_pe_time_) / total_time * 100 << " other="
			<< (total_time - stats_R_time_ - deductTime - stats_Z3_pe_time_)
					/ total_time * 100 << std::endl;
	std::cout << "=================================================="
			<< std::endl;

	if (writeSketch_)
		sketchFile_.close();

	return code;
}

std::vector<int> Synthesizer::generateSketch(SketchEncoding* sketch) {

	if (ngram_) {
		if (ngramSketches_.empty()) {
			sketch_.clear();
			return sketch_;
		}

		std::string key = ngramSketches_.front();
		//std::cout << "key=" << key << std::endl;

		assert(
				string2sketch_.find(ngramSketches_.front())
						!= string2sketch_.end());
		if ((inputFrames_.size() > 1
				&& key.find("inner_join") == std::string::npos)
				|| (numeric_comp_ && key.find("mutate") == std::string::npos
						&& key.find("summarise") == std::string::npos &&
						key.find("separate") == std::string::npos)) {
			assert(!string2sketch_[ngramSketches_.front()].empty());
			string2sketch_[ngramSketches_.front()].pop_back();
			ngramSketches_.pop_front();
			return generateSketch(sketch);
		} else {
			assert(!string2sketch_[ngramSketches_.front()].empty());
			sketch_ = string2sketch_[ngramSketches_.front()].back();
			string2sketch_[ngramSketches_.front()].pop_back();
			ngramSketches_.pop_front();
		}
	} else {
		sketch->getSketch(sketch_);
//	std::cout << "Sketch: ";
//	for (const auto& s : sketch_) {
//		std::cout << "[" << s << "]";
//	}
//	std::cout << std::endl;
	}

	return sketch_;
}

std::vector<std::string> Synthesizer::fillSketch(bool cacher, bool pe) {

	SketchFill * sketch_completion = new SketchFill(cacher, pe);

	sketch_completion->setOutputValues(output_double_, output_char_);

	std::vector<std::string> result = sketch_completion->fill(&deduct_,
			benchmark_, &R_, &predicate_, spec_map, dataframes_);

	stats_predicate_rejected_ +=
			sketch_completion->getStatsPredicatedRejected();
	stats_predicate_accepted_ +=
			sketch_completion->getStatsPredicatedAccepted();
	stats_pruned_ += sketch_completion->getStatsPredicatesPruned();
	stats_R_ += sketch_completion->getStatsRCalls();
	stats_R_time_ += sketch_completion->getRTime();
	stats_Z3_pe_time_ += sketch_completion->getZ3Time();
	stats_cached_R_ += sketch_completion->getStatsCached();

	//std::cout << "Iterations=" << sketch_completion->getIterations() << std::endl;

	delete sketch_completion;

	return result;

}

std::vector<Rcpp::DataFrame> Synthesizer::getInputDataFrames() {
	std::vector<Rcpp::DataFrame> vec_;
	for (unsigned int i = 0; i < benchmark_->getInput().size(); i++) {
		std::string input = benchmark_->getInput()[i];
		stats_R_++;
		int t1 = clock();
		Rcpp::DataFrame ds = R_.parseEvalNT(input);
		int t2 = clock();
		stats_R_time_ += Util::timeDiff(t1, t2);
		vec_.push_back(ds);
	}
	return vec_;
}

std::string Synthesizer::sketch2String(std::vector<int> skt) {
	std::string str = "";
	for (std::vector<int>::reverse_iterator i = skt.rbegin(); i != skt.rend();
			++i) {
		int id = *i;
		if ((id != 0) && (id != -1)) {
			Component* comp = this->spec_map[id];
			std::string comp_str = comp->getName();
			str += (comp_str + " ");
		}
	}
	return str;
}

void Synthesizer::loadNgram() {

	std::ifstream file(sketchNgram_);
	std::string str;
	while (std::getline(file, str)) {
		ngramSketches_.push_back(str);
	}
	file.close();
}

void Synthesizer::loadSketchMapping() {

	std::ifstream file;
	file.open(sketchMappingFilename_);
	std::string output;
	std::string key = "";
	std::string skt = "";
	bool flag = true;
	std::vector<int> tmp;

	if (file.is_open()) {
		while (!file.eof()) {
			file >> output;
			//std::cout << "out=" << output << " ";

			if (!output.compare("M")) {
				std::vector<int> tmp2(tmp.begin(), tmp.end());
				if (!tmp2.empty())
					string2sketch_[key].push_back(tmp2);
				tmp.clear();
				flag = true;
				key = "";
				skt = "";
			} else {

				if (!output.compare("v="))
					flag = false;
				else {

					if (flag) {
						if (key.compare(""))
							key += " ";
						key += output;
					} else {
						tmp.push_back(stoi(output));
						//map[key].push_back(stoi(output));
						skt += " ";
						skt += output;
					}
				}
			}

		}
	}
	file.close();
}

Synthesizer::~Synthesizer() {
	// TODO Auto-generated destructor stub
}

