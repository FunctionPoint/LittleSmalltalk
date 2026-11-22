#pragma once

/*
	Compiler
	Compiles Smalltalk source files into an CompiledImage, in two phases:
	1. compileSource reads the files with its Parser, line by line: first
	   every class definition line of all files (compileClass), then the
	   methods (a MethodCompiler each: bytecodes and literals), into
	   CompiledClass and CompiledMethod objects. No image objects are made
	   yet.
	2. generateImage builds the classes, metaclasses and methods in the
	   image from them: first the classes in hierarchy order (depth first,
	   the subclasses of a class sorted by name), then the methods class by
	   class in that order (class methods first, each side in source
	   order). The image does not depend on the order of the files.
	A syntax error stops lst with the file and line.
*/

#include <stddef.h>
#include <string>
#include <vector>

#include "CompiledClass.h"
#include "CompiledLiteral.h"
#include "CompiledMethod.h"
#include "Parser.h"

class Object;
class CompiledImage;

class Compiler
{
private:
	Parser parser;
	CompiledImage& image;

	std::vector<CompiledClass> classes;
	std::vector<CompiledClass*> hierarchyOrder;
	int methodLine = 0;

public:
	explicit Compiler( CompiledImage& image );
	Compiler( const Compiler& ) = delete;
	Compiler& operator=( const Compiler& ) = delete;

	void compileFolders( const std::vector<std::string>& sourceFolders );

private:
	void compileFiles( const std::vector<std::string>& sourceFiles );

	/* ---------- phase 1: source to CompiledClass and CompiledMethod ---------- */
	void compileSource( const char* sourceFile, bool classDefinitions );
	void compileLine( const char* sourceFile, bool classDefinitions );
	void checkClasses();
	void addSubclasses( const std::string& superclassName, std::vector<CompiledClass*>& order );

	CompiledClass* findClass( const std::string& name );
	bool isGlobal( const char* name );

	void compileClass();
	void compileMethodDefinition( bool classSide );

	/* class definition line */
	static bool skipSpaces( const char* line, size_t* start );
	static bool matchString( const char* line, size_t* start, const char* matchString );
	static std::string getWord( const char* line, size_t* end );
	static bool matchVarList( const char* line, size_t* end, std::string& variableList );
	static bool compileClassDefinition( const char* text, CompiledClass& _class );
	void compileVariableNames( const std::string& variableList, std::vector<std::string>& names );

	/* instance variables */
	std::vector<std::string> instanceVariablesOf( const CompiledClass* _class, bool classSide );
	static const std::vector<std::string>& variablesOf( const CompiledClass* _class, bool classSide );
	bool superclassOf( const CompiledClass*& _class, bool& classSide );

	/* ---------- phase 2: CompiledClass and CompiledMethod to image objects ---------- */
	void generateImage();
	void generateClass( CompiledClass& _class );
	void generateMethod( CompiledClass& _class, const CompiledMethod& method, bool classSide );
	Object* buildVariableArray( const std::vector<std::string>& names );
	Object* buildLiteral( const CompiledLiteral& literal );
	Object* buildLiteralArray( const std::vector<CompiledLiteral>& literals );
};
