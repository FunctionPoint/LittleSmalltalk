#pragma once

/*
	SpecialObjects
	The objects the VM knows by name: nil, true, false, the globals
	dictionary, the classes it creates instances of, and the symbols it
	sends. They are found in the globals after the image is read
	(findInGlobals), and registered as static roots, so the garbage
	collector updates them when it moves the objects: always read them
	from here, never keep a copy across an allocation.
*/

#include "Object.h"

class ObjectMemory;

class SpecialObjects
{
public:
	/* commonly used objects */
	Object* badMethodSymbol = nullptr;
	Object* binaryMessages[ 3 ] = {};
	Object* falseObject = nullptr;
	Object* globalsObject = nullptr;
	Object* initialMethod = nullptr;
	Object* nilObject = nullptr;
	Object* trueObject = nullptr;

	/* commonly used classes */
	Object* ArrayClass = nullptr;
	Object* BlockClass = nullptr;
	Object* ByteArrayClass = nullptr;
	Object* ContextClass = nullptr;
	Object* DictionaryClass = nullptr;
	Object* IntegerClass = nullptr;
	Object* SmallIntClass = nullptr;
	Object* StringClass = nullptr;
	Object* SymbolClass = nullptr;
	Object* UndefinedClass = nullptr;

	void findInGlobals( ObjectMemory& memory );

	Object* lookupGlobal( const char* name ) const;
	static Object* dictLookup( Object* dictionary, const char* name );

	Object* classOf( Object* object ) const;
	bool notNil( Object* object ) const;
};
