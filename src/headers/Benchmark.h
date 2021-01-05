/*
 * Benchmark.h
 *
 *  Created on: Aug 26, 2016
 *      Author: yu
 */

#ifndef MODEL_BENCHMARK_H_
#define MODEL_BENCHMARK_H_

#include <string>
#include <vector>
#include <stdexcept>
#include "document.h"

class Benchmark {

private:
	rapidjson::Document doc_;
	bool csv_ = false;
	std::string csvStr_;

public:
	Benchmark(const std::string& json);

	int getId();

	int getDepth();

	std::string getOutput();

	const std::vector<std::string>  getInput();

	//Binary represented by Vector
	const std::vector<int> getSketch();

	void dump();

	bool hasCsv();

	void setCsvStr(std::string s);

	std::string getCsvStr();

	void parseCsv(std::string s);

	virtual ~Benchmark();
};

#endif /* MODEL_BENCHMARK_H_ */
