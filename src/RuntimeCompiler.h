#pragma once

/*
	RuntimeCompiler
	Primitive 180: compiles the source text of a method in the running
	image, with the same MethodCompiler as lst -o, for Class>>parseMethod:
	(the REPL's expressions, File fileIn and Class>>addMethod:).
	The result is the material for a Method, built in the dynamic memory:
	an Array of
		1 the selector (a String)
		2 the bytecodes (a ByteArray)
		3 the literals (an Array of #( kind value ) pairs: kind 1 an object,
		  2 a symbol given as a String, 3 a literal Array given as an Array
		  of such pairs)
		4 the stack size (a SmallInt)
		5 the number of temporaries (a SmallInt)
	or, for a syntax error, a String with the message ("near line 2: ...").
	The Smalltalk side makes the symbols (Symbol new: keeps the symbol table)
	and the Method object.
*/

#include <string>
#include <vector>

#include "Parser.h"

class Object;
class ObjectMemory;
class SpecialObjects;
class Primitives;
class CompiledLiteral;
class CompiledMethod;

class RuntimeCompiler
{
private:
	ObjectMemory& memory;
	SpecialObjects& objects;
	Primitives& primitives;
	Parser parser;

public:
	RuntimeCompiler( ObjectMemory& memory, SpecialObjects& objects, Primitives& primitives );
	RuntimeCompiler( const RuntimeCompiler& ) = delete;
	RuntimeCompiler& operator=( const RuntimeCompiler& ) = delete;

	Object* compile( Object* text, Object* instanceVariables, bool& failed );

private:
	Object* buildResult( const CompiledMethod& method );
	Object* buildLiterals( const std::vector<CompiledLiteral>& literals );
	Object* buildLiteral( const CompiledLiteral& literal );
	Object* newString( const std::string& text );
	static std::string stringOf( Object* object );
};
