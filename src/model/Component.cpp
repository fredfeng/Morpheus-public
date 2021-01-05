/*
 * Component.cpp
 *
 *  Created on: Aug 27, 2016
 *      Author: yu
 */

#include "Component.h"
#include <string>
#include <vector>
#include <tuple>
#include "document.h"
#include <fstream>
#include <streambuf>
#include <iostream>

using namespace std;

Component::Component(const std::string& json) {
    std::ifstream t(json);
    std::string str((std::istreambuf_iterator<char>(t)),
                     std::istreambuf_iterator<char>());

	if (doc_.Parse<0>(str.c_str()).HasParseError())
		throw std::invalid_argument("json parse error");
}

int Component::getId() {
	return doc_["id"].GetInt();
}

string Component::getName() {
	return doc_["name"].GetString();
}

const vector<string> Component::getArgTypes() {
	rapidjson::Value& value = doc_["arguments"];
	if (!value.IsArray())
		throw std::logic_error("bad arguments");
	std::vector<std::string> vec;
    for (std::size_t i = 0; i < value.Size(); i++)
    	vec.push_back(value[i].GetString());

	return vec;
}

const vector<string> Component::getConstraint() {
	rapidjson::Value& value = doc_["constraint"];
	if (!value.IsArray())
		throw std::logic_error("bad constraints");
	vector<string> vec;
	for (std::size_t i = 0; i < value.Size(); i++) {
		vec.push_back(value[i].GetString());
	}

	return vec;
}

const string Component::getTemplate() {
	rapidjson::Value& value = doc_["template"];
	return value.GetString();
}


Component::~Component() {
	// TODO Auto-generated destructor stub
}
