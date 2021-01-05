/*
 * DummySynthesizer.h
 *
 *  Created on: Aug 26, 2016
 *      Author: yu
 */

#ifndef DEDUCTION_SYNTHESIZER_H_
#define DEDUCTION_SYNTHESIZER_H_

#include <vector>
#include <string>
#include <Component.h>
#include <SketchEncoding.h>
#include <Benchmark.h>
#include <Deduction.h>
#include <RInside.h>                    // for the embedded R via RInside
#include <Predicate.h>
#include <unordered_map>
#include <fstream>
#include <set>

class Synthesizer {

private:
	std::vector<int> sketch_;
	Benchmark* benchmark_;
	Deduction deduct_;
	std::unordered_map<int, Component*> spec_map;
	RInside R_;              // create an embedded R instance
	Predicate predicate_;
	bool forceSketch_;
	bool cache_R_;
	bool partialevaluation_;
	bool writeSketch_;
	std::string sketchFilename_;
	std::ofstream sketchFile_;
	std::string sketchMappingFilename_;
	std::string sketchNgram_;

	bool numeric_comp_;

	unsigned int stats_predicate_rejected_;
	unsigned int stats_predicate_accepted_;
//	unsigned int stats_r_tests_;
//	unsigned int stats_na_;
	unsigned int stats_pruned_;
	unsigned int stats_R_;
	double stats_R_time_;
	double stats_Z3_pe_time_;
	unsigned stats_cached_R_;

	int min_length_;
	int max_length_;

	bool ngram_;

	std::vector<Rcpp::DataFrame> inputFrames_;

	std::vector<Rcpp::DataFrame*> dataframes_;

	std::vector<std::multiset<double>> output_double_;
	std::vector<std::multiset<std::string>> output_char_;

	void printDataFrame(Rcpp::DataFrame *df) {
		predicate_.printDataFrame(df);
	}

	std::unordered_map<std::string, std::vector<std::vector<int> > > string2sketch_;
	std::deque<std::string> ngramSketches_;

public:
	Synthesizer(Benchmark* b, const char* spec_loc, int min, int max,
			bool chacher = false, bool forceskt = false, bool pe = true);

	std::vector<std::string> synthesis();

	std::vector<int> generateSketch(SketchEncoding* sketch);

	std::vector<std::string> fillSketch(bool cacher = false, bool pe = true);

	std::vector<Rcpp::DataFrame> getInputDataFrames();

	std::string sketch2String(std::vector<int> skt);

	Rcpp::DataFrame getOutputFrame() {
		std::string output = benchmark_->getOutput();
		stats_R_++;
		int t1 = clock();
		Rcpp::DataFrame ds = R_.parseEvalNT(output);
		int t2 = clock();
		stats_R_time_ += Util::timeDiff(t1, t2);
		return ds;
	}

	void setSketchFile(std::string filename) {
		writeSketch_ = true;
		sketchFilename_ = filename;
	}

	void setSketchMapping(std::string filename) {
		sketchMappingFilename_ = filename;
		loadSketchMapping();
	}

	void setNgramFile(std::string filename) {
		sketchNgram_ = filename;
	}

	void loadSketchMapping();

	void loadNgram();

	void setNgram(bool flag) {
		ngram_ = flag;
	}

	virtual ~Synthesizer();
};

#endif /* DEDUCTION_SYNTHESIZER_H_ */
