#include "Compiler.h"

#include <algorithm>
#include <filesystem>
#include <limits.h>
#include <string.h>

#include "Log.h"
#include "ObjectLayout.h"
#include "Object.h"
#include "Bytecode.h"
#include "CompiledClass.h"
#include "CompiledLiteral.h"
#include "CompiledMethod.h"
#include "CompiledImage.h"
#include "MethodCompiler.h"
#include "Parser.h"

Compiler::Compiler( CompiledImage& image )
	: image( image )
{
}

void Compiler::compileFolders( const std::vector<std::string>& sourceFolders )
{
	std::vector<std::string> sourceFiles;

	for( const std::string& folder : sourceFolders ) {
		std::error_code error;
		if( !std::filesystem::is_directory( folder, error ) )
			Log::error( "Source folder \"%s\" not found (-o takes folders, not files)", folder.c_str() );

		/* the files of the folder and of its subfolders */
		std::vector<std::filesystem::path> files;
		for( const std::filesystem::directory_entry& entry : std::filesystem::recursive_directory_iterator( folder, error ) )
			if( entry.is_regular_file( error ) && entry.path().extension() == ".st" )
				files.push_back( entry.path() );
		if( error )
			Log::error( "Cannot read source folder \"%s\": %s", folder.c_str(), error.message().c_str() );
		if( files.empty() )
			Log::error( "No Smalltalk source files (*.st) in source folder \"%s\"", folder.c_str() );

		std::sort( files.begin(), files.end() );
		for( const std::filesystem::path& file : files )
			sourceFiles.push_back( file.string() );
	}

	compileFiles( sourceFiles );
}

void Compiler::compileFiles( const std::vector<std::string>& sourceFiles )
{
	/* first the class definitions of all files, so the methods can use every class */
	for( const std::string& sourceFile : sourceFiles )
		compileSource( sourceFile.c_str(), true );
	checkClasses();

	for( const std::string& sourceFile : sourceFiles )
		compileSource( sourceFile.c_str(), false );
	generateImage();
}

/* ---------- phase 1: source to CompiledClass and CompiledMethod ---------- */

void Compiler::compileSource( const char* sourceFile, bool classDefinitions )
{
	parser.open( sourceFile );

	/* then read the image source file */
	while( parser.nextLine() ) {
		try {
			compileLine( sourceFile, classDefinitions );
		} catch( const CompileError& error ) {
			/* in a method: its first line is the one after the =Name or !Name line */
			int line = methodLine > 0 ? methodLine + error.line - 1 : parser.line();
			Log::error( "%s, line %d: %s", sourceFile, line, error.message.c_str() );
		}
	}
}

void Compiler::compileLine( const char* sourceFile, bool classDefinitions )
{
	methodLine = 0;
	switch( parser.current() ) {
	case '+':
		if( classDefinitions ) {
			parser.trimLine();
			parser.advance();
			compileClass();
		}
		break;
	case '=':
	case '!':
		if( classDefinitions )
			/* skip the method */
			parser.readMethodText();
		else {
			bool classSide = parser.current() == '=';
			parser.trimLine();
			parser.advance();
			compileMethodDefinition( classSide );
		}
		break;
	case 0: /* end of the line */
		break;
	case '"': /* nothing to do, get the next line. */
		break;
	default:
		Log::error( "Character %x ('%c') not a supported action!  Found in line \"%s\" (%d) of input file %s.", parser.current(), parser.current(), parser.text(), parser.line(), sourceFile );
		break;
	}
}

void Compiler::compileClass()
{
	CompiledClass _class;

	/*
	 * parse lines like:
	 * +SuperClass subclass: #NewClass variables: #( instVars ) classVariables: #( classVars )
	 */
	if( !compileClassDefinition( parser.text(), _class ) )
		parser.parseError( "Unable to parse class creation line!" );

	compileVariableNames( _class.classVariables, _class.classVariableNames );
	compileVariableNames( _class.instanceVariables, _class.instanceVariableNames );

	/* a class defined again replaces the earlier definition (the methods are compiled later) */
	CompiledClass* existing = findClass( _class.className );
	if( existing )
		*existing = std::move( _class );
	else
		classes.push_back( std::move( _class ) );
}

bool Compiler::compileClassDefinition( const char* text, CompiledClass& _class )
{
	size_t start = 0;
	size_t end = 0;

	if( text[ start ] != '+' )
		return false;

	/* get the superclass name */
	start++; /* skip the + char */
	end = start;

	_class.superclassName = getWord( text, &end );
	if( _class.superclassName.empty() )
		return false;

	start = end;

	/* now skip the whitespace */
	skipSpaces( text, &start );

	/* match the string "subclass:" */
	if( !matchString( text, &start, "subclass:" ) )
		return false;

	/* skip any spaces */
	skipSpaces( text, &start );

	end = start;

	/* get the new class name, as a symbol */
	_class.className = getWord( text, &end );
	if( _class.className.empty() )
		return false;

	start = end;

	/* skip any spaces */
	skipSpaces( text, &start );

	/* match the string "variables:" */
	if( !matchString( text, &start, "variables:" ) )
		return false;

	skipSpaces( text, &start );

	/* get the instance var list */
	if( !matchVarList( text, &start, _class.instanceVariables ) )
		return false;

	/* trim the leading spaces. */
	skipSpaces( text, &start );

	/* match the string "classVariables:" */
	if( !matchString( text, &start, "classVariables:" ) )
		return false;

	skipSpaces( text, &start );

	/* get the class var list */
	if( !matchVarList( text, &start, _class.classVariables ) )
		return false;

	return true;
}

std::string Compiler::getWord( const char* line, size_t* end )
{
	size_t start = *end;

	/* skip whitespace */
	skipSpaces( line, &start );

	/* skip leading '#' chars. */
	if( line[ start ] == '#' )
		start++;

	*end = start;

	/* read until we hit space or nul */
	while( line[ *end ] != ' ' && line[ *end ] != 0 )
		( *end )++;

	std::string result( &line[ start ], *end - start );

	return result;
}

bool Compiler::skipSpaces( const char* line, size_t* start )
{
	size_t oldStart = *start;

	while( line[ *start ] == ' ' )
		( *start )++;

	/* true if there were spaces to skip */
	return oldStart != *start;
}

bool Compiler::matchString( const char* line, size_t* start, const char* matchString )
{
	size_t i = 0;

	while( line[ *start + i ] == matchString[ i ] )
		i++;

	if( i != strlen( matchString ) )
		return false;
	*start += i;
	return true;
}

bool Compiler::matchVarList( const char* line, size_t* end, std::string& variableList )
{
	size_t start = *end;
	size_t listStart = start;
	size_t listEnd = start;

	skipSpaces( line, &start );

	/* match '#' */
	if( line[ start ] != '#' )
		return false;

	start++;

	/* match '(' */
	if( line[ start ] != '(' )
		return false;

	start++;

	/* skip early spaces */
	skipSpaces( line, &start );

	listStart = start;

	/* find the end */
	*end = start;

	while( line[ *end ] != ')' && line[ *end ] != 0 )
		( *end )++;

	if( line[ *end ] == 0 )
		return false;

	/* so line[*end] == ')' */
	listEnd = ( *end ) - 1;

	if( listEnd <= listStart )
		listEnd = listStart;

	variableList.assign( &line[ listStart ], listEnd - listStart );

	/* bump end past the ')' character */
	( *end )++;

	return true;
}

void Compiler::compileVariableNames( const std::string& variableList, std::vector<std::string>& names )
{
	names.clear();
	if( variableList.empty() )
		return;

	/* copy the variable list into the input buffer. */
	parser.setText( variableList.c_str() );

	/* Now parse the variable names */
	while( parser.current() ) {
		parser.skipSpaces();
		if( !Parser::isIdentifierChar( parser.current() ) )
			break;
		parser.readIdentifier();

		names.push_back( parser.token() );
	}
}

CompiledClass* Compiler::findClass( const std::string& name )
{
	for( CompiledClass& _class : classes )
		if( _class.className == name )
			return &_class;
	return nullptr;
}

void Compiler::checkClasses()
{
	/* the superclass must exist: a class of the image (the big bang) or one of the source files */
	for( const CompiledClass& _class : classes )
		if( !isGlobal( _class.superclassName.c_str() ) )
			Log::error( "Unable to find class %s, the superclass of %s!", _class.superclassName.c_str(), _class.className.c_str() );

	hierarchyOrder.clear();
	addSubclasses( "", hierarchyOrder );

	/* a class not reached from a root is in a cycle of superclasses */
	if( hierarchyOrder.size() != classes.size() ) {
		for( CompiledClass& _class : classes )
			if( std::find( hierarchyOrder.begin(), hierarchyOrder.end(), &_class ) == hierarchyOrder.end() )
				Log::error( "Class %s is its own superclass!", _class.className.c_str() );
	}
}

bool Compiler::isGlobal( const char* name )
{
	if( image.lookupGlobal( name, true ) )
		return true;
	for( const CompiledClass& _class : classes )
		if( _class.className == name || _class.metaclassName() == name )
			return true;
	return false;
}

void Compiler::addSubclasses( const std::string& superclassName, std::vector<CompiledClass*>& order )
{
	/* the subclasses of superclassName, or with "" the roots: the classes whose superclass is not compiled */
	std::vector<CompiledClass*> subclasses;
	for( CompiledClass& _class : classes ) {
		bool isSubclass = superclassName.empty() ? !findClass( _class.superclassName ) : _class.superclassName == superclassName;
		if( isSubclass )
			subclasses.push_back( &_class );
	}
	std::sort( subclasses.begin(), subclasses.end(), []( const CompiledClass* left, const CompiledClass* right ) { return left->className < right->className; } );

	for( CompiledClass* _class : subclasses ) {
		order.push_back( _class );
		addSubclasses( _class->className, order );
	}
}

void Compiler::compileMethodDefinition( bool classSide )
{
	const char* command = classSide ? "ClassMethodCommand()" : "InstanceMethodCommand()";

	/* read class name */
	parser.readIdentifier();

	CompiledClass* _class = findClass( parser.token() );
	if( !_class )
		Log::error( "%s: unknown class in Method %s!", command, parser.token() );

	methodLine = parser.line() + 1;
	parser.readMethodText();

	parser.rewind();
	parser.skipSpaces();

	MethodCompiler methodCompiler( parser, instanceVariablesOf( _class, classSide ), [ this ]( const char* name ) { return isGlobal( name ); } );
	CompiledMethod method = methodCompiler.compile();

	if( classSide )
		_class->classMethods.push_back( std::move( method ) );
	else
		_class->instanceMethods.push_back( std::move( method ) );
}

std::vector<std::string> Compiler::instanceVariablesOf( const CompiledClass* _class, bool classSide )
{
	/* the classes from _class up, then their variables from the top down */
	std::vector<std::pair<const CompiledClass*, bool>> chain;
	do {
		chain.push_back( { _class, classSide } );
	} while( superclassOf( _class, classSide ) );

	std::vector<std::string> names;
	for( auto link = chain.rbegin(); link != chain.rend(); ++link ) {
		const std::vector<std::string>& variables = variablesOf( link->first, link->second );
		names.insert( names.end(), variables.begin(), variables.end() );
	}
	return names;
}

bool Compiler::superclassOf( const CompiledClass*& _class, bool& classSide )
{
	if( classSide && _class->metaclassName() == "MetaObject" ) {
		/* the metaclass of Object has Class as its parent */
		_class = findClass( "Class" );
		classSide = false;
	} else
		/* the parent of a metaclass is the metaclass of the superclass */
		_class = findClass( _class->superclassName );
	return _class != nullptr;
}

const std::vector<std::string>& Compiler::variablesOf( const CompiledClass* _class, bool classSide )
{
	return classSide ? _class->classVariableNames : _class->instanceVariableNames;
}

/* ---------- phase 2: CompiledClass and CompiledMethod to image objects ---------- */

void Compiler::generateImage()
{
	/* the classes first, in hierarchy order */
	for( CompiledClass* _class : hierarchyOrder )
		generateClass( *_class );

	/* then the methods, class by class in the same order: the class methods, then the instance methods */
	for( CompiledClass* _class : hierarchyOrder ) {
		for( const CompiledMethod& method : _class->classMethods )
			generateMethod( *_class, method, true );
		for( const CompiledMethod& method : _class->instanceMethods )
			generateMethod( *_class, method, false );
	}
}

void Compiler::generateClass( CompiledClass& _class )
{
	Object* ClassClass = image.lookupGlobal( "Class", false );
	Object* MetaObjectClass = image.lookupGlobal( "MetaObject", false );
	int classVariableCount = (int) _class.classVariableNames.size();

	/* look up the superclass */
	_class.superclass = image.lookupGlobal( _class.superclassName.c_str(), false );

	/* make the metaclass first. */
	std::string metaclassName = _class.metaclassName();

	_class.metaclass = image.lookupGlobal( metaclassName.c_str(), true );
	if( !_class.metaclass ) {
		_class.metaclass = image.newClass( metaclassName.c_str(), 0 );
		image.addGlobal( metaclassName.c_str(), _class.metaclass );
	}

	/* set up the class tree, weird for metaclasses. */
	_class.metaclass->objectClass = ClassClass;
	if( _class.metaclass != MetaObjectClass )
		_class.metaclass->data[ ObjectLayout::parentClassInClass ] = _class.superclass->objectClass;
	else
		_class.metaclass->data[ ObjectLayout::parentClassInClass ] = ClassClass;

	/* the class vars are the instance variables of the metaclass */
	Object* classVariableArray = buildVariableArray( _class.classVariableNames );

	/* fix up as much as possible of the metaclass. */
	_class.metaclass->data[ ObjectLayout::instanceSizeInClass ] = SmallInt::make( classVariableCount + ObjectLayout::classSize );
	_class.metaclass->data[ ObjectLayout::variablesInClass ] = classVariableArray;
	_class.metaclass->data[ ObjectLayout::methodsInClass ] = image.newDictionary();

	/* now make the new class. */
	_class.instanceClass = image.lookupGlobal( _class.className.c_str(), true );
	if( !_class.instanceClass ) {
		_class.instanceClass = image.newClass( _class.className.c_str(), classVariableCount );
		image.addGlobal( _class.className.c_str(), _class.instanceClass );
	}

	/* get the instance vars */
	Object* instanceVariableArray = buildVariableArray( _class.instanceVariableNames );
	int instanceSize = (int) _class.instanceVariableNames.size();

	/* class Object has a superclass of nil */
	if( _class.superclass && _class.superclass != image.nil() )
		instanceSize += SmallInt::value( _class.superclass->data[ ObjectLayout::instanceSizeInClass ] );

	/* fix up as much as possible of the class. */
	_class.instanceClass->data[ ObjectLayout::parentClassInClass ] = _class.superclass;
	_class.instanceClass->data[ ObjectLayout::instanceSizeInClass ] = SmallInt::make( instanceSize );
	_class.instanceClass->data[ ObjectLayout::variablesInClass ] = instanceVariableArray;
	_class.instanceClass->data[ ObjectLayout::methodsInClass ] = image.newDictionary();
	_class.instanceClass->objectClass = _class.metaclass;
}

Object* Compiler::buildVariableArray( const std::vector<std::string>& names )
{
	std::vector<Object*> variables;

	for( const std::string& name : names )
		variables.push_back( image.newSymbol( name.c_str() ) );

	if( variables.empty() )
		return image.nil();

	Object* result = image.alloc( (int) variables.size() );
	result->objectClass = image.lookupGlobal( "Array", false );
	for( size_t i = 0; i < variables.size(); i++ )
		result->data[ i ] = variables[ i ];
	return result;
}

void Compiler::generateMethod( CompiledClass& _class, const CompiledMethod& method, bool classSide )
{
	Object* methodClass = classSide ? _class.metaclass : _class.instanceClass;
	Object* stMethod = image.alloc( ObjectLayout::methodSize );

	stMethod->objectClass = image.lookupGlobal( "Method", false );

	/* the selector before the literals: the order of the symbols matters for the image */
	stMethod->data[ ObjectLayout::nameInMethod ] = image.newSymbol( method.selector.c_str() );
	stMethod->data[ ObjectLayout::literalsInMethod ] = buildLiteralArray( method.literals );
	stMethod->data[ ObjectLayout::byteCodesInMethod ] = method.code.buildByteArray( image );
	stMethod->data[ ObjectLayout::stackSizeInMethod ] = SmallInt::make( method.stackSize );
	stMethod->data[ ObjectLayout::temporarySizeInMethod ] = SmallInt::make( method.variableCount );
	stMethod->data[ ObjectLayout::classInMethod ] = methodClass;

	image.dictionaryInsert( methodClass->data[ ObjectLayout::methodsInClass ], stMethod->data[ ObjectLayout::nameInMethod ], stMethod );
}

Object* Compiler::buildLiteralArray( const std::vector<CompiledLiteral>& literals )
{
	std::vector<Object*> objects;

	/* make the literals in order: the order of the symbols matters for the image */
	for( const CompiledLiteral& literal : literals )
		objects.push_back( buildLiteral( literal ) );

	if( objects.empty() )
		return image.nil();

	Object* result = image.alloc( (int) objects.size() );
	result->objectClass = image.lookupGlobal( "Array", false );
	for( size_t i = 0; i < objects.size(); i++ )
		result->data[ i ] = objects[ i ];
	return result;
}

Object* Compiler::buildLiteral( const CompiledLiteral& literal )
{
	Object* object = image.nil();

	switch( literal.kind ) {
	case CompiledLiteral::Integer:
		/* as the image always had them: SmallInt::make of an int */
		if( literal.value < INT_MIN || literal.value > INT_MAX )
			Log::error( "integer literal %lld is too large for the image", (long long) literal.value );
		object = SmallInt::make( (int) literal.value );
		break;
	case CompiledLiteral::String:
		object = image.newString( literal.text.c_str() );
		break;
	case CompiledLiteral::Symbol:
		object = image.newSymbol( literal.text.c_str() );
		break;
	case CompiledLiteral::Char:
		object = image.alloc( 1 );
		object->objectClass = image.lookupGlobal( "Char", false );
		object->data[ 0 ] = SmallInt::make( (int) literal.value );
		break;
	case CompiledLiteral::Global:
		object = image.lookupGlobal( literal.text.c_str(), false );
		break;
	case CompiledLiteral::Array:
		object = buildLiteralArray( literal.elements );
		if( object == image.nil() ) {
			/* #( ): an empty Array, not nil */
			object = image.alloc( 0 );
			object->objectClass = image.lookupGlobal( "Array", false );
		}
		break;
	}
	return object;
}
