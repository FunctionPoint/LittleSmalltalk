#include <stddef.h>

#include "Log.h"
#include "ObjectLayout.h"
#include "ObjectMemory.h"
#include "SpecialObjects.h"

void SpecialObjects::findInGlobals( ObjectMemory& memory )
{
	nilObject = lookupGlobal( "nil" );
	memory.addStaticRoot( &nilObject );

	trueObject = lookupGlobal( "true" );
	memory.addStaticRoot( &trueObject );

	falseObject = lookupGlobal( "false" );
	memory.addStaticRoot( &falseObject );

	ArrayClass = lookupGlobal( "Array" );
	memory.addStaticRoot( &ArrayClass );

	BlockClass = lookupGlobal( "Block" );
	memory.addStaticRoot( &BlockClass );

	ByteArrayClass = lookupGlobal( "ByteArray" );
	memory.addStaticRoot( &ByteArrayClass );

	ContextClass = lookupGlobal( "Context" );
	memory.addStaticRoot( &ContextClass );

	DictionaryClass = lookupGlobal( "Dictionary" );
	memory.addStaticRoot( &DictionaryClass );

	IntegerClass = lookupGlobal( "Integer" );
	memory.addStaticRoot( &IntegerClass );

	SmallIntClass = lookupGlobal( "SmallInt" );
	memory.addStaticRoot( &SmallIntClass );

	StringClass = lookupGlobal( "String" );
	memory.addStaticRoot( &StringClass );

	SymbolClass = lookupGlobal( "Symbol" );
	memory.addStaticRoot( &SymbolClass );

	UndefinedClass = lookupGlobal( "Undefined" );
	memory.addStaticRoot( &UndefinedClass );
}

Object* SpecialObjects::lookupGlobal( const char* name ) const
{
	return dictLookup( globalsObject, name );
}

Object* SpecialObjects::dictLookup( Object* dictionary, const char* name )
{
	if( !dictionary )
		Log::error( "Called with NULL dictionary oop!" );

	Object* keys = dictionary->data[ ObjectLayout::keysInDictionary ];
	int low = 0;
	int high = keys->size();

	/*
	 * Do a binary search through its keys, which are Symbols.
	 */
	while( low < high ) {
		int middle = ( low + high ) / 2;
		Object* key = keys->data[ middle ];

		/*
		 * If we find the key, return the
		 * object.
		 */

		int result = Symbol::compare( name, key );

		if( result == 0 ) {
			Object* values = dictionary->data[ ObjectLayout::valuesInDictionary ];
			return values->data[ middle ];
		} else {
			if( result < 0 )
				high = middle;
			else
				low = middle + 1;
		}
	}

	return NULL;
}

Object* SpecialObjects::classOf( Object* object ) const
{
	return SmallInt::is( object ) ? SmallIntClass : object->objectClass;
}

bool SpecialObjects::notNil( Object* object ) const
{
	return object && ( object != nilObject );
}
