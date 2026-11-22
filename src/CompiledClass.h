#pragma once

/*
	CompiledClass
	The result of compiling one class in the first phase of the Compiler:
	its definition line
	+SuperClass subclass: #NewClass variables: #( instVars ) classVariables: #( classVars )
	and its methods. The second phase builds the class and metaclass
	objects from it and stores them here too.
*/

#include <string>
#include <vector>

#include "CompiledMethod.h"

class Object;

class CompiledClass
{
public:
	/* the parts of "+Super subclass: #Name variables: #( a b ) classVariables: #( c )" */
	std::string superclassName;
	std::string className;
	std::string instanceVariables;
	std::string classVariables;

	/* the same variables, one name each */
	std::vector<std::string> instanceVariableNames;
	std::vector<std::string> classVariableNames;

	std::vector<CompiledMethod> instanceMethods;
	std::vector<CompiledMethod> classMethods;

	/* the objects built in the second phase */
	Object* superclass = nullptr;
	Object* metaclass = nullptr;
	Object* instanceClass = nullptr;

	std::string metaclassName() const;
};
