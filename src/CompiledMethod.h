#pragma once

/*
	CompiledMethod
	The result of compiling one method in the first phase of the Compiler:
	the selector, the bytecodes, the literals and the
	number of variables. No image objects yet: the literals (CompiledLiteral) only
	say what to make, and the second phase creates the Method object from it.
*/

#include <string>
#include <vector>

#include "Bytecode.h"
#include "CompiledLiteral.h"

class CompiledMethod
{
public:
	std::string selector;
	Bytecode code;
	std::vector<CompiledLiteral> literals;
	int variableCount = 0;
	int stackSize = 0;
};
