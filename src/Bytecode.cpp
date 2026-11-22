#include "Bytecode.h"

#include <stddef.h>

#include "Log.h"
#include "Object.h"
#include "CompiledImage.h"

void Bytecode::clear()
{
	bytes.clear();
}

const std::vector<uint8_t>& Bytecode::byteCodes() const
{
	return bytes;
}

int Bytecode::byteTop() const
{
	return (int) bytes.size();
}

void Bytecode::genByte( int byte )
{
	bytes.push_back( (uint8_t) byte );
}

void Bytecode::genVal( int value )
{
	if( ( value < 0 ) || ( value > 0xFFFF ) )
		Log::error( "genVal(): illegal value %d!", value );
	genByte( value & 0xFF );
	genByte( value >> 8 );
}

void Bytecode::genValPos( int position, int value )
{
	if( ( value < 0 ) || ( value > 0xFFFF ) )
		Log::error( "genValPos() illegal value %d!", value );
	bytes[ position ] = (uint8_t) ( value & 0xFF );
	bytes[ position + 1 ] = (uint8_t) ( value >> 8 );
}

void Bytecode::genInstruction( int opcode, int argument )
{
	/*printf("gen instruction %d %d\n", a, b); */
	if( argument < 16 )
		genByte( opcode * 16 + argument );
	else {
		genInstruction( 0, opcode );
		genByte( argument );
	}
}

Object* Bytecode::buildByteArray( CompiledImage& image ) const
{
	ByteObject* byteArray = image.allocBytes( byteTop() );
	for( size_t i = 0; i < bytes.size(); i++ )
		byteArray->bytes[ i ] = bytes[ i ];
	byteArray->objectClass = image.lookupGlobal( "ByteArray", false );
	return (Object*) byteArray;
}
