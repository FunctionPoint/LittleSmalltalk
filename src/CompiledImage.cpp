#include "CompiledImage.h"

#include <stdio.h>
#include <string.h>

#include "Log.h"
#include "ObjectLayout.h"
#include "Image.h"
#include "Object.h"
#include "ObjectMemory.h"

CompiledImage::CompiledImage( ObjectMemory& memory )
	: memory( memory )
{
	/* big bang -- create the first classes */
	bigBang();
}

void CompiledImage::bigBang( void )
{
	/*
	 * First, make the nil (undefined) object;
	 * notice its class is wrong
	 */
	objects.nilObject = alloc( 0 );

	/*
	 * Second, make classes for Symbol and Dictionary.
	 * this will allow newClass to work correctly
	 */

	objects.SymbolClass = alloc( ObjectLayout::classSize + 1 );
	addGlobal( "Symbol", objects.SymbolClass );
	objects.SymbolClass->data[ ObjectLayout::nameInClass ] = newSymbol( "Symbol" );

	objects.DictionaryClass = alloc( ObjectLayout::classSize );
	addGlobal( "Dictionary", objects.DictionaryClass );
	objects.DictionaryClass->data[ ObjectLayout::nameInClass ] = newSymbol( "Dictionary" );

	/* now we can fix up nil's class */
	objects.UndefinedClass = newClass( "Undefined", 0 );
	addGlobal( "Undefined", objects.UndefinedClass );
	objects.nilObject->objectClass = objects.UndefinedClass;
	addGlobal( "nil", objects.nilObject );

	/* make up the object / metaobject mess */
	Object* ObjectClass = newClass( "Object", 0 );
	addGlobal( "Object", ObjectClass );
	Object* MetaObjectClass = newClass( "MetaObject", 0 );
	addGlobal( "MetaObject", MetaObjectClass );
	ObjectClass->objectClass = MetaObjectClass;
	ObjectClass->data[ ObjectLayout::parentClassInClass ] = objects.nilObject;

	/* And the Class/MetaClass mess */
	Object* ClassClass = newClass( "Class", 0 );
	addGlobal( "Class", ClassClass );
	Object* MetaClassClass = newClass( "MetaClass", 0 );
	addGlobal( "MetaClass", MetaClassClass );
	ClassClass->objectClass = MetaClassClass;

	/* one more tweak. This is needed to stop lookups. */
	MetaObjectClass->data[ ObjectLayout::parentClassInClass ] = ClassClass;

	/* now make up a bunch of other classes */
	objects.BlockClass = newClass( "Block", 0 );
	addGlobal( "Block", objects.BlockClass );

	/* SmallInt has an extra class variable, just like Symbol */
	objects.SmallIntClass = newClass( "SmallInt", 1 );
	addGlobal( "SmallInt", objects.SmallIntClass );

	objects.IntegerClass = newClass( "Integer", 0 );
	addGlobal( "Integer", objects.IntegerClass );

	Object* TrueClass = newClass( "True", 0 );
	addGlobal( "True", TrueClass );
	objects.trueObject = alloc( 0 );
	objects.trueObject->objectClass = TrueClass;
	addGlobal( "true", objects.trueObject );

	Object* FalseClass = newClass( "False", 0 );
	addGlobal( "False", FalseClass );
	objects.falseObject = alloc( 0 );
	objects.falseObject->objectClass = FalseClass;
	addGlobal( "false", objects.falseObject );

	objects.ArrayClass = newClass( "Array", 0 );
	addGlobal( "Array", objects.ArrayClass );

	objects.ByteArrayClass = newClass( "ByteArray", 0 );
	addGlobal( "ByteArray", objects.ByteArrayClass );

	Object* OrderedArrayClass = newClass( "OrderedArray", 0 );
	addGlobal( "OrderedArray", OrderedArrayClass );

	objects.StringClass = newClass( "String", 0 );
	addGlobal( "String", objects.StringClass );

	Object* TreeClass = newClass( "Tree", 0 );
	addGlobal( "Tree", TreeClass );

	/* finally, we can fill in the fields in class Object */
	ObjectClass->data[ ObjectLayout::methodsInClass ] = newDictionary();
	ObjectClass->data[ ObjectLayout::instanceSizeInClass ] = SmallInt::make( 0 );
	ClassClass->data[ ObjectLayout::instanceSizeInClass ] = SmallInt::make( 0 );

	/* can make global name, but can't fill it in */
	objects.globalsObject = alloc( 2 );
	addGlobal( "globals", objects.globalsObject );
}

Object* CompiledImage::alloc( int size )
{
	/* no class yet: checkGlobals finds the classes that were never defined */
	Object* result = memory.allocateStatic( size );
	result->objectClass = nullptr;

	while( size > 0 )
		result->data[ --size ] = objects.nilObject;

	return result;
}

void CompiledImage::addGlobal( const char* name, Object* value )
{
	globalNames.push_back( name );
	globalObjects.push_back( value );
}

Object* CompiledImage::newSymbol( const char* text )
{
	/* first see if it is already a symbol */
	for( Object* symbol : oldSymbols )
		if( Symbol::compare( text, symbol ) == 0 )
			return symbol;

	/* not there, make a new one */
	ByteObject* result = allocBytes( (int) strlen( text ) );
	for( int i = 0; i < (int) strlen( text ); i++ )
		result->bytes[ i ] = (uint8_t) text[ i ];
	result->objectClass = lookupGlobal( "Symbol", false );
	oldSymbols.push_back( (Object*) result );
	return (Object*) result;
}

ByteObject* CompiledImage::allocBytes( int size )
{
	int wordCount = Object::toWords( size );
	ByteObject* result = (ByteObject*) alloc( wordCount );
	result->setSize( size );
	result->setBinary();
	return result;
}

Object* CompiledImage::lookupGlobal( const char* name, bool okMissing )
{
	for( size_t i = 0; i < globalNames.size(); i++ )
		if( globalNames[ i ] == name )
			return globalObjects[ i ];
	/* not found, return 0 */
	if( !okMissing )
		Log::error( "lookupGlobalName(): Missing global %s!", name );
	return 0;
}

Object* CompiledImage::newClass( const char* name, int variableCount )
{
	Object* classObject = alloc( ObjectLayout::classSize + variableCount );
	classObject->data[ ObjectLayout::nameInClass ] = newSymbol( name );

	return classObject;
}

Object* CompiledImage::newDictionary( void )
{
	Object* result = alloc( 2 );
	result->objectClass = lookupGlobal( "Dictionary", false );
	result->data[ 0 ] = newOrderedArray();
	result->data[ 1 ] = newArray( 0 );
	return result;
}

/*
 * newOrderedArray()
 *	Return a new, empty ordered array
 */
Object* CompiledImage::newOrderedArray( void )
{
	Object* result = alloc( 0 );
	result->objectClass = lookupGlobal( "OrderedArray", false );
	return ( result );
}

/*
 * newArray()
 *	Allocate a new array
 *
 * All slots are initialized to nil
 */
Object* CompiledImage::newArray( int size )
{
	Object* result = alloc( size );
	result->objectClass = lookupGlobal( "Array", false );
	for( int i = 0; i < size; ++i )
		result->data[ i ] = objects.nilObject;
	return ( result );
}

Object* CompiledImage::nil() const
{
	return objects.nilObject;
}

Object* CompiledImage::newString( const char* text )
{
	size_t size = strlen( text );
	ByteObject* newObject = allocBytes( (int) size );
	for( size_t i = 0; i < size; i++ )
		newObject->bytes[ i ] = (uint8_t) text[ i ];
	newObject->objectClass = lookupGlobal( "String", false );
	return (Object*) newObject;
}

void CompiledImage::dictionaryInsert( Object* dictionary, Object* key, Object* value )
{
	Object *keys = dictionary->data[ ObjectLayout::keysInDictionary ], *values = dictionary->data[ ObjectLayout::valuesInDictionary ];
	int i, limit;

	/*
	 * Scan the OrderedArray "keys" to find where we fit in
	 */
	for( i = 0, limit = keys->size(); i < limit; ++i ) {
		int comparison = Symbol::compare( key, keys->data[ i ] );

		/*
		 * We should go in before this node
		 */
		if( comparison < 0 ) {
			dictionary->data[ 0 ] = insert( keys, i, key );
			dictionary->data[ 1 ] = insert( values, i, value );
			return;
		} else if( comparison > 0 )
			continue;
		else
			Log::error( "dictionaryInsert(): duplicate key" );
	}

	/*
	 * The new element goes at the end
	 */
	dictionary->data[ 0 ] = insert( keys, i, key );
	dictionary->data[ 1 ] = insert( values, i, value );
}

/*
 * insert()
 *	Insert an element in the array at the given position
 *
 * Creates a new Array-ish object of the same class as "array",
 * and returns it filled in as requested.
 */
Object* CompiledImage::insert( Object* array, int position, Object* value )
{
	/*
	 * Clone the current object, including class.  Make one
	 * extra slot in the Array storage.
	 */
	Object* grownArray = alloc( array->size() + 1 );
	grownArray->objectClass = array->objectClass;

	int i;
	/*
	 * Copy up to the index
	 */
	for( i = 0; i < position; ++i )
		grownArray->data[ i ] = array->data[ i ];

	/*
	 * Put in the new element at this position
	 */
	int oldIndex = i;
	grownArray->data[ i++ ] = value;

	/*
	 * Now copy the rest
	 */
	for( ; oldIndex < (int) array->size(); ++oldIndex )
		grownArray->data[ i++ ] = array->data[ oldIndex ];
	return ( grownArray );
}

int CompiledImage::writeImage( const char* outputFile )
{
	Object* specialSymbols = NULL;
	int objectCount = 0;

	/* set up special symbols */
	specialSymbols = newArray( 4 );
	specialSymbols->data[ 0 ] = newSymbol( "<" );
	specialSymbols->data[ 1 ] = newSymbol( "<=" );
	specialSymbols->data[ 2 ] = newSymbol( "+" );
	specialSymbols->data[ 3 ] = newSymbol( "doesNotUnderstand:" );

	/* add important objects to globals for later lookup. */
	if( !lookupGlobal( "specialSymbols", true ) )
		addGlobal( "specialSymbols", specialSymbols );

	if( !lookupGlobal( "nil", true ) )
		addGlobal( "nil", objects.nilObject );

	if( !lookupGlobal( "true", true ) )
		addGlobal( "true", objects.trueObject );

	if( !lookupGlobal( "false", true ) )
		addGlobal( "false", objects.falseObject );

	/* then create the tree of symbols */
	objects.SymbolClass->data[ ObjectLayout::symbolsInSymbol ] = buildSymbolTree();

	/* fix up globals. */
	fillGlobalsDictionary( objects.globalsObject );

	/* see if anything was never defined in the class source */
	checkGlobals();

	FILE* file = fopen( outputFile, "wb" );
	if( file == NULL )
		Log::error( "Unable to open \"%s\" for writing!", outputFile );

	objectCount = Image( memory, objects ).write( file, objects.globalsObject );

	fclose( file );

	return objectCount;
}

Object* CompiledImage::buildSymbolTree( void )
{
	Object* tree = newTree();
	for( Object* symbol : oldSymbols )
		tree->data[ 0 ] = symbolTreeInsert( tree->data[ 0 ],
											newNode( symbol, objects.nilObject,
													 objects.nilObject ) );
	return tree;
}

Object* CompiledImage::newTree( void )
{
	Object* result = alloc( 1 );
	result->objectClass = lookupGlobal( "Tree", false );
	return result;
}

Object* CompiledImage::newNode( Object* value, Object* left, Object* right )
{
	Object* result = alloc( 3 );
	result->objectClass = lookupGlobal( "Node", false );
	result->data[ 0 ] = value;
	result->data[ 1 ] = left;
	result->data[ 2 ] = right;
	return result;
}

Object* CompiledImage::symbolTreeInsert( Object* base, Object* symbolNode )
{
	if( base == objects.nilObject )
		return symbolNode;
	if( Symbol::compare( symbolNode->data[ ObjectLayout::valueInNode ], base->data[ ObjectLayout::valueInNode ] ) < 0 )
		base->data[ ObjectLayout::leftInNode ] =
			symbolTreeInsert( base->data[ ObjectLayout::leftInNode ], symbolNode );
	else
		base->data[ ObjectLayout::rightInNode ] =
			symbolTreeInsert( base->data[ ObjectLayout::rightInNode ], symbolNode );
	return base;
}

void CompiledImage::fillGlobalsDictionary( Object* dictionary )
{
	dictionary->objectClass = lookupGlobal( "Dictionary", false );
	dictionary->data[ 0 ] = newOrderedArray();
	dictionary->data[ 1 ] = newArray( 0 );

	/*
	 * Insert each class name as a reference to the class
	 * object itself.
	 */
	for( size_t i = 0; i < globalNames.size(); i++ ) {
		if( globalNames[ i ].starts_with( "Meta" ) )
			continue;

		dictionaryInsert( dictionary, newSymbol( globalNames[ i ].c_str() ), globalObjects[ i ] );
	}

	/*
	 * Insert this object itself under the name "Smalltalk"
	 */
	dictionaryInsert( dictionary, newSymbol( "Smalltalk" ), dictionary );
}

void CompiledImage::checkGlobals( void )
{
	for( size_t i = 0; i < globalObjects.size(); i++ )
		if( !globalObjects[ i ]->objectClass )
			Log::error( "checkGlobals(): Class %s never defined!", globalNames[ i ].c_str() );
}
