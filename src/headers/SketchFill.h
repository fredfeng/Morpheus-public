#ifndef DEDUCTION_SKETCHFILL_H_
#define DEDUCTION_SKETCHFILL_H_

#include <Deduction.h>
#include <Benchmark.h>
#include <RInside.h>
#include <Predicate.h>

#include <vector>
#include <map>
#include <unordered_map>
#include <string>

class SketchFill {

public:
	SketchFill(bool cacher, bool pe) {
		stats_predicate_accepted_ = 0;
		stats_predicate_rejected_ = 0;
		stats_pruned_ = 0;
		stats_R_ = 0;
		stats_R_time_ = 0;
		stats_Z3_pe_time_ = 0;
		stats_cached_ = 0;
		enable_cache_ = cacher;
		partialevaluation_ = pe;
		itn_limit_ = 0;
	}
	;

	~SketchFill() {
		for (unsigned i = 0; i < cache_store_.size(); i++) {
			delete cache_store_[i];
			cache_store_[i] = NULL;
		}
	}

	std::vector<std::string> fill(Deduction *deduct, Benchmark *benchmark,
			RInside *R, Predicate *predicate,
			std::unordered_map<int, Component*> &specs,
			std::vector<Rcpp::DataFrame*>& dataframes);

	unsigned int getIterations(){
		return itn_limit_;
	}

	unsigned int getStatsPredicatedAccepted() {
		return stats_predicate_accepted_;
	}

	unsigned int getStatsPredicatedRejected() {
		return stats_predicate_rejected_;
	}

	unsigned int getStatsPredicatesPruned() {
		return stats_pruned_;
	}

	unsigned int getStatsRCalls() {
		return stats_R_;
	}

	double getRTime() {
		return stats_R_time_;
	}

	double getZ3Time() {
		return stats_Z3_pe_time_;
	}

	unsigned int getStatsCached() {
		return stats_cached_;
	}

	void setOutputValues(std::vector<std::multiset<double>>& out_d, std::vector<std::multiset<std::string>> out_c){
		output_double_ = out_d;
		output_char_ = out_c;
	}

protected:
	enum State {
		init,
		decision,
		deduction,
		next,
		test,
		conflict,
		backtrack,
		sat,
		unsat,
		end
	};

	unsigned int stats_predicate_rejected_;
	unsigned int stats_predicate_accepted_;
	unsigned int stats_pruned_;
	unsigned int stats_R_;
	unsigned int stats_cached_;

	std::vector<std::multiset<double>> output_double_;
	std::vector<std::multiset<std::string>> output_char_;

	bool enable_cache_;
	bool partialevaluation_;

	double stats_Z3_pe_time_;
	double stats_R_time_;

	std::string cmd_;
	std::vector<std::string> extra_code;
	std::vector<std::string> gather_str_;

	unsigned itn_limit_;

	std::unordered_map<std::string, int> cache_code_;
	std::vector<Rcpp::DataFrame*> cache_store_;

	std::string synthesizeLine(Component * comp,
			std::vector<std::string>& inputs, std::string output,
			std::vector<std::string>& predicate);

	bool passTest(std::vector<std::string>& code, Rcpp::DataFrame *df,
			Benchmark *benchmark, RInside *R,
			std::vector<Rcpp::DataFrame*>& dataframes,
			Rcpp::DataFrame *df_out);

	bool renameDataFrame(const std::string &output,
			const std::string &synthesize, std::vector<std::string> &code,
			RInside *R);

	void getCode(Node * root, std::vector<std::string>& code);

	int getInputTables(Component * comp);

	bool pruneTables(RInside * R, std::string name, Rcpp::DataFrame *df);

	int getNumGroups(int node_id, RInside * R){

		std::string line = "n_groups(TBL_"  + std::to_string(node_id) + ")";
		int result = runRint(R,line);

		return result;
	}

	std::set<std::string> getCharGroups(int node_id, RInside * R){

		std::set<std::string> char_groups;
		std::string line = "as.character(groups(TBL_" + std::to_string(node_id) + "))";
		Rcpp::CharacterVector rv = runR(R,line);
		for (unsigned i = 0; i < rv.size(); i++)
			char_groups.insert(std::string(rv[i]));

		return char_groups;
	}

	bool isUnifiable(Deduction * deduct, bool pe = true) {

		if (!pe)
			return true;

		int t1 = clock();
		bool res = deduct->solve();
		int t2 = clock();
		stats_Z3_pe_time_ += Util::timeDiff(t1, t2);

		return res;
	}

	std::string convertRtoString(Deduction * deduct, RInside *R,
			std::string cmd);

	SEXP runR(RInside *R, std::string cmd) {
		int t1 = clock();
		stats_R_++;
		SEXP result = (SEXP) R->parseEval(cmd);
		int t2 = clock();
		stats_R_time_ += Util::timeDiff(t1, t2);
		return result;
	}

	int runRint(RInside *R, std::string cmd) {
		int t1 = clock();
		stats_R_++;
		int result = (int) R->parseEval(cmd);
		int t2 = clock();
		stats_R_time_ += Util::timeDiff(t1, t2);
		return result;
	}

	SEXP runRNT(RInside *R, std::string cmd) {
		int t1 = clock();
		stats_R_++;
		SEXP result = (SEXP) R->parseEvalNT(cmd);
		int t2 = clock();
		stats_R_time_ += Util::timeDiff(t1, t2);
		return result;
	}

	bool runRNTBoolean(RInside *R, std::string cmd) {
		int t1 = clock();
		stats_R_++;
		bool result = R->parseEvalNT(cmd);
		int t2 = clock();
		stats_R_time_ += Util::timeDiff(t1, t2);
		return result;
	}

	void runRQ(RInside *R, std::string cmd) {
		int t1 = clock();
		stats_R_++;
		R->parseEvalQ(cmd);
		int t2 = clock();
		stats_R_time_ += Util::timeDiff(t1, t2);
	}

	void runRQNT(RInside *R, std::string cmd) {
		int t1 = clock();
		stats_R_++;
		R->parseEvalNT(cmd);
		int t2 = clock();
		stats_R_time_ += Util::timeDiff(t1, t2);
	}

	std::string getState(State s) {

		std::string output = "";

		switch (s) {
		case init:
			output = "init";
			break;
		case decision:
			output = "decision";
			break;
		case deduction:
			output = "deduction";
			break;
		case next:
			output = "next";
			break;
		case test:
			output = "test";
			break;
		case conflict:
			output = "conflict";
			break;
		case backtrack:
			output = "backtrack";
			break;
		case sat:
			output = "sat";
			break;
		case unsat:
			output = "unsat";
			break;
		case end:
			output = "end";
			break;
		}

		return output;
	}

};

#endif
