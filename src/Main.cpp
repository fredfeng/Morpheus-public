//============================================================================
// Name        : Main.cpp
// Author      : 
// Version     :
// Copyright   : Your copyright notice
// Description : Hello World in C++, Ansi-style
//============================================================================

#include "Benchmark.h"
#include <iostream>
#include <RInside.h>                    // for the embedded R via RInside
#include <iostream>
#include <chrono>
#include <ctime>

#include <string>
#include <fstream>
#include <streambuf>
#include "SketchTree.h"
#include <glob.h>
#include <Synthesizer.h>
#include <optionparser.h>

using namespace std;

using option::Option;
using option::Descriptor;
using option::Parser;
using option::Stats;
using option::ArgStatus;

enum optionIndex {
	UNKNOWN, HELP, FIXSKETCH, CACHE_R, PARTIALEVAL, BENCHMARK, SPECS, LB, UB, WRITESKETCH, NGRAM, SMAP
};

struct Arg: public option::Arg
{
  static ArgStatus Required(const Option& option, bool)
  {
    return option.arg == 0 ? option::ARG_ILLEGAL : option::ARG_OK;
  }
  static ArgStatus Empty(const Option& option, bool)
  {
    return (option.arg == 0 || option.arg[0] == 0) ? option::ARG_OK : option::ARG_IGNORE;
  }
};

const option::Descriptor usage[] = {
		{ UNKNOWN, 0, "", "", Arg::None, "USAGE: ./Morpheus [options]\n\n""Options:" },
		{ HELP, 0, "", "help", Arg::None, "  --help  \tPrint usage and exit." },
		{ FIXSKETCH, 0, "", "fixsketch", Arg::None, "  --fixsketch  \tEnables the fixing of the sketch." },
		{ CACHE_R, 0, "", "cacher", Arg::None, "  --cacher \tEnables caching for R programs." },
		{ PARTIALEVAL, 0, "", "disablepe", Arg::None, "  --disablepe  \tDisables partial evaluation." },
		{ BENCHMARK, 0, "", "benchmark", Arg::Required, "  --benchmark <file>  \tBenchmark .json file." },
		{ SPECS, 0, "", "specs", Arg::Required, "  --specs <dir> \tDirectory for specs." },
		{ LB, 0, "","lb", Arg::Required, "  --lb <num> \tLower bound for the size of the solution." },
		{ UB, 0, "","ub", Arg::Required,"  --ub <num> \tUpper bound for the size of the solution." },
		{ WRITESKETCH, 0, "","writesketch", Arg::Required,"  --writesketch <file> \tWrite all sketches to a file." },
		{ SMAP, 0, "","sketchmap", Arg::Required,"  --sketchmap <file> \tRead sketch to vector mapping from file." },
		{ NGRAM, 0, "","ngram", Arg::Required,"  --ngram <file> \tRead sketches from re-ranked ngram file." },
		{ 0, 0, 0, 0, 0, 0 } };

int main(int argc, char *argv[]) {

	argc -= (argc > 0);
	argv += (argc > 0); // skip program name argv[0] if present
	option::Stats stats(usage, argc, argv);
	option::Option * options = new option::Option[stats.options_max];
	option::Option * buffer = new option::Option[stats.buffer_max];
	option::Parser parse(usage, argc, argv, options, buffer);

	if (parse.error())
		return 1;

	if (options[HELP] || argc == 0) {
		option::printUsage(std::cout, usage);
		return 0;
	}

	if (options[BENCHMARK].count() == 0 || options[SPECS].count() == 0) {
		cout << "Please specify the json file of your benchmark!"
				<< "\n e.g., ./Morpheus --benchmark ../benchmarks/1/input.json --specs \"/home/yu/research/Morpheus/specs/L1/*\"\n";
		return 0;
	}

	int lb = 1;
	int ub = 5;

	if (options[LB].count())
		lb = atoi(options[LB].arg);

	if (options[UB].count())
		ub = atoi(options[UB].arg);

	assert (lb >= 1 && ub >= lb);

	std::cout << "Loading benchmark: " << options[BENCHMARK].arg << std::endl;
	std::cout << "Specs: " << options[BENCHMARK].arg << std::endl;
	std::cout << "Searching between lengths [" << lb << "," << ub
			<< "]" << std::endl;

	bool forceskt = (options[FIXSKETCH].count() == 1);
	bool cacher = (options[CACHE_R].count() == 1);
	bool pe = !(options[PARTIALEVAL].count() == 1);

	std::cout << "Fixing sketch: " << forceskt << std::endl;
	std::cout << "Caching R programs: " << cacher << std::endl;
	std::cout << "Partial evaluation: " << pe << std::endl;

	Benchmark benchmark(std::string(options[BENCHMARK].arg));
	Synthesizer synthesizer(&benchmark, options[SPECS].arg, lb, ub, cacher, forceskt, pe);

	if (options[NGRAM].count()) {
		std::cout << "Ngram re-reranked sketch file: " << options[NGRAM].arg
				<< std::endl;
		synthesizer.setNgramFile(options[NGRAM].arg);
		synthesizer.loadNgram();
		synthesizer.setNgram(true);
		assert (options[SMAP].count());
	}


	if (options[SMAP].count()) {
		std::cout << "Mapping from string to vector file: " << options[SMAP].arg
						<< std::endl;
		synthesizer.setSketchMapping(std::string(options[SMAP].arg));
		assert (options[NGRAM].count());
	}

	if (options[WRITESKETCH].count()){
			std::cout << "Writing sketches to file: " << options[WRITESKETCH].arg << std::endl;
			synthesizer.setSketchFile(std::string(options[WRITESKETCH].arg));
	}


	std::vector<std::string> code = synthesizer.synthesis();

		if (code.empty()) {
			cout << "R program not found!" << endl;
		} else {
			cout << "R program: " << endl;
			for (const auto& line : code) {
				cout << line << endl;
			}
		}

	delete options;
	delete buffer;

	int value = 10;
	if (code.empty()) value = 20;

	return value;
}
