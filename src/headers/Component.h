/*
 * Component.h
 *
 *  Created on: Aug 27, 2016
 *      Author: yu
 *
//	{
//	  "id":8,
//	  "name":"cbind",
//	  "arguments":
//	  [
//	    "Table",
//	    "Table"
//	  ],
//	  "constraints":
//	  [
//	    ["left": "CO", "operator": "==", "right": "CI1+CI2"],
//	    ["left": "RO", "operator": "==", "right": "RI1"],
//	    ["left": "RO", "operator": "==", "right": "RI2"]
//	  ]
//	}
 */

#ifndef MODEL_COMPONENT_H_
#define MODEL_COMPONENT_H_

#include <string>
#include <vector>
#include "document.h"

class Component {

private:
	rapidjson::Document doc_;

public:
	Component(const std::string& json);

	int getId();

	std::string getName();

	const std::vector<std::string> getArgTypes();

	const std::vector<std::string> getConstraint();

	const std::string getTemplate();

	virtual ~Component();
};

#endif /* MODEL_COMPONENT_H_ */
