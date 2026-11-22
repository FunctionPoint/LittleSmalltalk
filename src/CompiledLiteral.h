#pragma once

/*
	CompiledLiteral
	A literal of a CompiledMethod, from the first phase of the Compiler:
	only what to make. The second phase makes the object from it (or the
	RuntimeCompiler, for primitive 180).
*/

#include <stdint.h>
#include <string>
#include <vector>

class CompiledLiteral
{
public:
	enum Kind {
		Integer, /* value */
		String,	 /* text */
		Symbol,	 /* text */
		Char,	 /* value */
		Global,	 /* text: the name of the global */
		Array	 /* elements: a literal array #( ... ) */
	};

	Kind kind = Integer;
	int64_t value = 0;
	std::string text;
	std::vector<CompiledLiteral> elements;
};
