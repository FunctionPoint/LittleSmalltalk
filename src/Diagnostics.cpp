#include <iomanip>
#include <iostream>
#include <string>
#include <string_view>

#include "Diagnostics.h"
#include "ObjectLayout.h"
#include "ObjectMemory.h"
#include "SpecialObjects.h"

Diagnostics::Diagnostics( const ObjectMemory& memory, const SpecialObjects& objects )
	: memory( memory ), objects( objects )
{
}

void Diagnostics::backTrace( Object* aContext ) const
{
	std::cout << "back trace\n";

	/* check if we actually got a Context. */
	if( !objects.notNil( aContext ) )
		return;
	if( !memory.isDynamic( aContext ) )
		return;
	/* a Block is a Context too: the interpreter runs a block with the Block as its context */
	if( aContext->objectClass != objects.ContextClass && aContext->objectClass != objects.BlockClass )
		return;

	while( objects.notNil( aContext ) ) {
		printContext( aContext );
		aContext = aContext->data[ ObjectLayout::previousContextInContext ];
	}
}

void Diagnostics::printContext( Object* aContext ) const
{
	Object* method = aContext->data[ ObjectLayout::methodInContext ];

	if( objects.notNil( method ) ) {
		Object* _class = method->data[ ObjectLayout::classInMethod ];

		if( objects.notNil( _class ) ) {
			Object* className = _class->data[ ObjectLayout::nameInClass ];

			if( objects.notNil( className ) )
				std::cout << "class " << textOf( className );
			else
				std::cout << "nil class name";
		} else
			std::cout << "nil class";

		Object* selector = method->data[ ObjectLayout::nameInMethod ];
		if( objects.notNil( selector ) )
			std::cout << " #" << textOf( selector ) << ' ';
		else
			std::cout << " nil symbol ";
	} else
		std::cout << " nil method ";

	Object* arguments = aContext->data[ ObjectLayout::argumentsInContext ];
	if( objects.notNil( arguments ) )
		printArguments( arguments );
	std::cout << '\n';
}

void Diagnostics::printArguments( Object* arguments ) const
{
	std::cout << '(';
	for( int i = 0; i < (int) arguments->size(); i++ ) {
		if( i > 0 )
			std::cout << ", ";
		printArgument( arguments->data[ i ] );
	}

	std::cout << ')';
}

void Diagnostics::printArgument( Object* argument ) const
{
	if( argument ) {
		if( SmallInt::is( argument ) )
			std::cout << SmallInt::value( argument );
		else {
			Object* _class = objects.classOf( argument );

			if( objects.notNil( _class ) ) {
				Object* symbol = _class->data[ ObjectLayout::nameInClass ];

				if( objects.notNil( symbol ) )
					std::cout << textOf( symbol );
				else
					std::cout << "nil class name";
			} else
				std::cout << "nil class";
		}
	} else
		std::cout << "nil arg";
}

void Diagnostics::dumpObject( Object* object, int indent ) const
{
	std::cerr << std::string( indent, ' ' );

	if( objects.classOf( object ) == objects.SmallIntClass )
		std::cerr << SmallInt::value( object );
	else if( objects.classOf( object ) == objects.UndefinedClass )
		std::cerr << "nil";
	else if( objects.classOf( object ) == objects.SymbolClass )
		std::cerr << '#' << textOf( object );
	else if( objects.classOf( object ) == objects.StringClass )
		std::cerr << '\'' << textOf( object ) << '\'';
	else if( objects.classOf( object ) == objects.ArrayClass ) {
		std::cerr << "#(";
		for( int i = 0; i < (int) object->size(); i++ )
			dumpObject( object->data[ i ], 1 );
		std::cerr << " )";
	} else if( object->isBinary() ) {
		std::cerr << textOf( object->objectClass->data[ ObjectLayout::nameInClass ] ) << " #(" << std::hex << std::setfill( '0' );
		for( int i = 0; i < (int) object->size(); i++ )
			std::cerr << ' ' << std::setw( 2 ) << (int) object->bytePtr()[ i ];
		std::cerr << std::dec << std::setfill( ' ' ) << " )";
	} else {
		Object* _class = objects.classOf( object );
		Object* name = _class->data[ ObjectLayout::nameInClass ];

		std::cerr << textOf( name );
	}
}

void Diagnostics::dumpMethod( Object* aMethod, int indent ) const
{
	if( !aMethod || aMethod == objects.nilObject )
		return;

	std::string fieldIndent( indent + 4, ' ' );

	std::cerr << std::string( indent, ' ' ) << "Method " << textOf( aMethod->data[ ObjectLayout::nameInMethod ] ) << '\n';

	/* bytecodes */
	std::cerr << fieldIndent << "Bytecodes: ";
	dumpObject( aMethod->data[ ObjectLayout::byteCodesInMethod ], 0 );
	std::cerr << '\n';

	/* literals */
	std::cerr << fieldIndent << "Literals: ";
	dumpObject( aMethod->data[ ObjectLayout::literalsInMethod ], 0 );
	std::cerr << '\n';

	/* stack size */
	std::cerr << fieldIndent << "Stack size: ";
	dumpObject( aMethod->data[ ObjectLayout::stackSizeInMethod ], 0 );
	std::cerr << '\n';

	/* temporary size */
	std::cerr << fieldIndent << "Temporary size: ";
	dumpObject( aMethod->data[ ObjectLayout::temporarySizeInMethod ], 0 );
	std::cerr << '\n';

	/* class of method */
	std::cerr << fieldIndent << "Class of method: ";
	dumpObject( aMethod->data[ ObjectLayout::classInMethod ], 0 );
	std::cerr << '\n';
}

void Diagnostics::dumpDictKeys( Object* dictionary )
{
	int keyCount = 0;

	if( !dictionary )
		return;

	Object* keys = dictionary->data[ ObjectLayout::keysInDictionary ];
	if( !keys )
		return;

	keyCount = keys->size();

	for( int i = 0; i < keyCount; i++ ) {
		Object* key = keys->data[ i ];

		if( key )
			std::cout << textOf( key ) << ' ';
	}
}

std::string_view Diagnostics::textOf( Object* object )
{
	return std::string_view( (const char*) object->bytePtr(), object->size() );
}
