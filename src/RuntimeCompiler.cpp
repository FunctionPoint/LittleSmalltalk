#include "RuntimeCompiler.h"

#include <stdint.h>
#include <string.h>

#include "CompiledLiteral.h"
#include "CompiledMethod.h"
#include "MethodCompiler.h"
#include "Object.h"
#include "ObjectMemory.h"
#include "Primitives.h"
#include "SpecialObjects.h"

/* the kinds of the #( kind value ) literal pairs */
static constexpr int literalObject = 1;
static constexpr int literalSymbol = 2;
static constexpr int literalArray = 3;

RuntimeCompiler::RuntimeCompiler( ObjectMemory& memory, SpecialObjects& objects, Primitives& primitives )
	: memory( memory ), objects( objects ), primitives( primitives )
{
}

Object* RuntimeCompiler::compile( Object* text, Object* instanceVariables, bool& failed )
{
	failed = false;
	if( SmallInt::is( text ) || !text->isBinary() || SmallInt::is( instanceVariables ) || instanceVariables->isBinary() ) {
		failed = true;
		return objects.nilObject;
	}

	/* copy the arguments first: allocating may move them */
	std::string source = stringOf( text );
	std::vector<std::string> names;
	for( uint32_t i = 0; i < instanceVariables->size(); i++ ) {
		Object* name = instanceVariables->data[ i ];
		if( SmallInt::is( name ) || !name->isBinary() ) {
			failed = true;
			return objects.nilObject;
		}
		names.push_back( stringOf( name ) );
	}

	CompiledMethod method;
	try {
		parser.setText( source.c_str() );
		parser.skipSpaces();
		MethodCompiler methodCompiler( parser, std::move( names ), [ this ]( const char* name ) { return objects.lookupGlobal( name ) != nullptr; } );
		method = methodCompiler.compile();
	} catch( const CompileError& error ) {
		return newString( "near line " + std::to_string( error.line ) + ": " + error.message );
	}

	return buildResult( method );
}

Object* RuntimeCompiler::buildResult( const CompiledMethod& method )
{
	Object* result = memory.allocate( 5 );
	result->objectClass = objects.ArrayClass;
	for( int i = 0; i < 5; i++ )
		result->data[ i ] = objects.nilObject;
	memory.pushRoot( result );

	Object* selector = newString( method.selector );
	memory.peekRoot()->data[ 0 ] = selector;

	const std::vector<uint8_t>& code = method.code.byteCodes();
	Object* byteCodes = memory.allocateBytes( (int) code.size() );
	byteCodes->objectClass = objects.ByteArrayClass;
	if( !code.empty() )
		memcpy( byteCodes->bytePtr(), code.data(), code.size() );
	memory.peekRoot()->data[ 1 ] = byteCodes;

	Object* literals = buildLiterals( method.literals );
	memory.peekRoot()->data[ 2 ] = literals;

	result = memory.popRoot();
	result->data[ 3 ] = SmallInt::make( method.stackSize );
	result->data[ 4 ] = SmallInt::make( method.variableCount );
	return result;
}

Object* RuntimeCompiler::buildLiterals( const std::vector<CompiledLiteral>& literals )
{
	Object* array = memory.allocate( (int) literals.size() );
	array->objectClass = objects.ArrayClass;
	for( size_t i = 0; i < literals.size(); i++ )
		array->data[ i ] = objects.nilObject;
	memory.pushRoot( array );

	for( size_t i = 0; i < literals.size(); i++ ) {
		Object* pair = buildLiteral( literals[ i ] );
		memory.peekRoot()->data[ i ] = pair;
	}
	return memory.popRoot();
}

Object* RuntimeCompiler::buildLiteral( const CompiledLiteral& literal )
{
	Object* value = objects.nilObject;
	int kind = literalObject;

	switch( literal.kind ) {
	case CompiledLiteral::Integer:
		if( literal.value >= INTPTR_MIN && literal.value <= INTPTR_MAX && SmallInt::fits( (intptr_t) literal.value ) )
			value = SmallInt::make( (intptr_t) literal.value );
		else
			value = primitives.newLInteger( literal.value );
		break;
	case CompiledLiteral::String:
		value = newString( literal.text );
		break;
	case CompiledLiteral::Symbol:
		kind = literalSymbol;
		value = newString( literal.text );
		break;
	case CompiledLiteral::Char:
		value = memory.allocate( 1 );
		value->objectClass = objects.lookupGlobal( "Char" );
		value->data[ 0 ] = SmallInt::make( (intptr_t) literal.value );
		break;
	case CompiledLiteral::Global:
		value = objects.lookupGlobal( literal.text.c_str() );
		if( !value )
			value = objects.nilObject;
		break;
	case CompiledLiteral::Array:
		kind = literalArray;
		value = buildLiterals( literal.elements );
		break;
	}

	memory.pushRoot( value );
	Object* pair = memory.allocate( 2 );
	pair->objectClass = objects.ArrayClass;
	pair->data[ 0 ] = SmallInt::make( kind );
	pair->data[ 1 ] = memory.popRoot();
	return pair;
}

Object* RuntimeCompiler::newString( const std::string& text )
{
	Object* string = memory.allocateBytes( (int) text.size() );
	string->objectClass = objects.StringClass;
	if( !text.empty() )
		memcpy( string->bytePtr(), text.data(), text.size() );
	return string;
}

std::string RuntimeCompiler::stringOf( Object* object )
{
	return std::string( (const char*) object->bytePtr(), object->size() );
}
