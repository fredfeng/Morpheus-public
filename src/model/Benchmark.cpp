/*
 * Benchmark.cpp
 *
 *  Created on: Aug 26, 2016
 *      Author: yu
 */

#include "Benchmark.h"
#include "document.h"
#include "Util.h"
#include <string>
#include <fstream>
#include <streambuf>
#include <iostream>
#include <vector>



using namespace rapidjson;

Benchmark::Benchmark(const std::string& json) {
	std::ifstream t(json);
	std::string str((std::istreambuf_iterator<char>(t)),
			std::istreambuf_iterator<char>());
	if (Util::contains(json, "|")) {
		parseCsv(json);
	} else {
		if (doc_.Parse<0>(str.c_str()).HasParseError())
			throw std::invalid_argument("json parse error");
	}

}

int Benchmark::getId() {
	return doc_["id"].GetInt();
}

int Benchmark::getDepth() {
	return doc_["depth"].GetInt();
}

std::string Benchmark::getOutput() {
	return doc_["output"].GetString();
}

const std::vector<std::string> Benchmark::getInput() {
	rapidjson::Value& value = doc_["input"];
	if (!value.IsArray())
		throw std::logic_error("bad input");
	std::vector<std::string> vec;
    for (std::size_t i = 0; i < value.Size(); i++)
    	vec.push_back(value[i].GetString());

	return vec;
}

const std::vector<int>  Benchmark::getSketch() {
	rapidjson::Value& value = doc_["sketch"];
	if (!value.IsArray())
		throw std::logic_error("bad sketch");
	std::vector<int> vec;
    for (std::size_t i = 0; i < value.Size(); i++)
    	vec.push_back(value[i].GetInt());

	return vec;
}

void Benchmark::dump() {
	std::cout << "----------------------------------\n";
	std::cout << "BenchmarkId:" << getId() << std::endl;
//	std::cout << "Depth:" << getDepth() << std::endl;
	std::cout << "Output table:" << getOutput() << std::endl;

	std::cout << "Input tables:";

    for (std::size_t i = 0; i < getInput().size(); i++)
        std::cout << getInput()[i] << " ";

	std::cout << "\n";

	std::cout << "Sketch (Components):";

    for (std::size_t i = 0; i < getSketch().size(); i++)
        std::cout << getSketch()[i] << " ";

	std::cout << "\n";
	std::cout << "----------------------------------\n";
}

bool Benchmark::hasCsv() {
	return csv_;
}

void Benchmark::setCsvStr(std::string s) {
	csvStr_ = s;
}

std::string Benchmark::getCsvStr() {
	return csvStr_;
}

void Benchmark::parseCsv(std::string str) {
	std::vector<std::string> vec;
	Util::split(str, '|', vec);
	csv_ = true;
	std::string val = "";
	std::string output = "";
	std::vector<std::string> input_vec;
	//iterate vector.
	for (unsigned int i = 0; i < vec.size(); i++) {
		size_t lastindex = vec[i].find_last_of(".");
		std::string path_name = vec[i].substr(0, lastindex);
		unsigned found = path_name.find_last_of("/\\");
		std::string rawname = path_name.substr(found + 1);
		if (i != (vec.size() - 1)) {
			input_vec.push_back(rawname);
//			std::cout << "input file: " + vec[i] << " rawname:" << rawname
//					<< std::endl;
		} else {
			output = rawname;
//			std::cout << "output file: " + vec[i] << " rawname:" << rawname << std::endl;
		}

		std::string script = rawname + " <- read.csv('" + vec[i]
				+ "', check.names = FALSE); fctr.cols <- sapply(" + rawname
				+ ", is.factor); int.cols <- sapply(" + rawname
				+ ", is.integer); " + rawname + "[, fctr.cols] <- sapply("
				+ rawname + "[, fctr.cols], as.character); " + rawname
				+ "[, int.cols] <- sapply(" + rawname
				+ "[, int.cols], as.numeric);";
		val += script + " ";
	}
	this->setCsvStr(val);

	//automatically generate json file.
	std::string js_str = "{\"id\":999,\"depth\":1,\"input\":["
			+ Util::vec2string2(input_vec) + "],\"output\":\"" + output
			+ "\",\"sketch\":[]}";
//	std::cout << "Final json: " + js_str << std::endl;

	if (doc_.Parse<0>(js_str.c_str()).HasParseError())
		throw std::invalid_argument("json parse error");

//	std::cout << "input:" << Util::vec2string2(input_vec) << std::endl;
//	std::cout << "output:" << output << std::endl;

}

Benchmark::~Benchmark() {
	// TODO Auto-generated destructor stub
}

