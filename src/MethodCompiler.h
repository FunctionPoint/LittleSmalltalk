#pragma once

/*
	MethodCompiler
	Parses the source text of one method (from the Parser's cursor on) and
	generates its bytecodes into a CompiledMethod: the selector, the code,
	the literals (only described, see CompiledLiteral) and the number of
	variables. It makes no objects, so both compilers use it: the Compiler
	(lst -o, the image being bootstrapped) and the RuntimeCompiler
	(primitive 180, the REPL and fileIn in a running image).
	What it needs to know about the class comes from outside: the names of
	the instance variables (all of them, superclass variables first) and
	whether a name is a global. A syntax error throws a CompileError (see
	Parser::parseError).
*/

#include <stdint.h>
#include <functional>
#include <string>
#include <vector>

#include "CompiledLiteral.h"
#include "CompiledMethod.h"
#include "Parser.h"

class MethodCompiler
{
private:
	Parser& parser;
	std::vector<std::string> instanceVariables;
	std::function<bool( const char* name )> isGlobal;

	CompiledMethod currentMethod;
	std::vector<std::string> argumentNames;
	std::vector<std::string> variableNames;
	bool superMessage = false;
	int returnOperation = 0;
	int blockbackup = -1;

public:
	MethodCompiler( Parser& parser, std::vector<std::string> instanceVariables, std::function<bool( const char* name )> isGlobal );
	MethodCompiler( const MethodCompiler& ) = delete;
	MethodCompiler& operator=( const MethodCompiler& ) = delete;

	CompiledMethod compile();

private:
	bool compileMethod();
	void resetMethod();

	/* literals */
	int addLiteral( const CompiledLiteral& literal );
	int addIntegerLiteral( int64_t value );
	int addStringLiteral( const char* text );
	int addSymbolLiteral( const char* text );
	int addCharLiteral( int value );
	int addGlobalLiteral( const char* name );
	CompiledLiteral readLiteralArray();
	CompiledLiteral readArrayElement();
	int readCharLiteral();

	void genInstruction( int opcode, int argument );

	/* names */
	void addArgument( const char* name );
	void addVariable( const char* name );
	int instanceIndex( const char* name ) const;
	int variableTop() const;

	/* compiling */
	bool compileInteger( void );
	bool compilePrimitive( void );
	bool compileString( void );
	bool nameTerm( const char* name );
	bool compileBlock( void );
	bool compileSymbol( void );
	bool compileChar( void );
	bool compileTerm( void );
	bool compileUnaryContinuation( void );
	bool compileBinaryContinuation( void );
	bool optimizeBlock( void );
	bool controlFlow( int branchOperation, const char* rest, int defaultConstant );
	bool optimizeLoop( int branchInstruction );
	bool compileKeywordContinuation( void );
	bool compileAssignment( const char* name );
	bool compileExpression( void );
	bool compileStatement( void );
	bool compileBody( void );
	bool compileMethodHeader();
	bool compileMethodVariables( void );
};
