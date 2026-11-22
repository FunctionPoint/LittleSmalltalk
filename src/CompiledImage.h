#pragma once

/*
	CompiledImage
	The image being compiled from a Smalltalk source file (lst -o): creates
	the first classes (the big bang), keeps the global names (classes,
	nil, true, false...) and their objects, allocates the objects in the
	static space of the ObjectMemory (they never move), keeps the table of
	all symbols, and writes the image file with Image. The Compiler fills
	it in. The VM then loads the image file it wrote.
*/

#include <string>
#include <vector>

#include "SpecialObjects.h"

class Object;
class ByteObject;
class ObjectMemory;

class CompiledImage
{
private:
	ObjectMemory& memory;

	std::vector<std::string> globalNames;
	std::vector<Object*> globalObjects;
	std::vector<Object*> oldSymbols;

	SpecialObjects objects;

public:
	explicit CompiledImage( ObjectMemory& memory );
	CompiledImage( const CompiledImage& ) = delete;
	CompiledImage& operator=( const CompiledImage& ) = delete;

	int writeImage( const char* outputFile );

	Object* nil() const;

	/* globals */
	void addGlobal( const char* name, Object* value );
	Object* lookupGlobal( const char* name, bool okMissing );

	/* objects */
	Object* alloc( int size );
	ByteObject* allocBytes( int size );

	Object* newString( const char* text );
	Object* newSymbol( const char* text );
	Object* newClass( const char* name, int variableCount );
	Object* newDictionary( void );
	Object* newArray( int size );
	Object* newOrderedArray( void );

	void dictionaryInsert( Object* dictionary, Object* key, Object* value );

private:
	void bigBang( void );

	void fillGlobalsDictionary( Object* dictionary );
	void checkGlobals( void );

	Object* newNode( Object* value, Object* left, Object* right );
	Object* newTree( void );
	Object* insert( Object* array, int position, Object* value );
	Object* symbolTreeInsert( Object* base, Object* symbolNode );
	Object* buildSymbolTree( void );
};
