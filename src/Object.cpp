#include "Object.h"

#include <limits.h>
#include <string.h>

/* ---------- ObjectHeader ---------- */

uint32_t ObjectHeader::size( uintptr_t header )
{
	return (uint32_t) header >> 2;
}

uintptr_t ObjectHeader::withSize( uint32_t size )
{
	return (uintptr_t) ( size << 2 );
}

/* ---------- Object ---------- */

uint32_t Object::size() const
{
	return ObjectHeader::size( header );
}

void Object::setSize( uint32_t size )
{
	header = ObjectHeader::withSize( size );
}

bool Object::isBinary() const
{
	return ( header & ObjectHeader::binaryFlag ) != 0;
}

void Object::setBinary()
{
	header |= ObjectHeader::binaryFlag;
}

bool Object::isGcDone() const
{
	return ( header & ObjectHeader::gcDoneFlag ) != 0;
}

void Object::setGcDone()
{
	header |= ObjectHeader::gcDoneFlag;
}

uint8_t* Object::bytePtr()
{
	return ( (ByteObject*) this )->bytes;
}

int Object::toWords( int byteCount )
{
	return ( byteCount + BytesPerWord - 1 ) / BytesPerWord;
}

Object* Object::wordsUp( void* pointer, intptr_t words )
{
	return (Object*) ( ( (char*) pointer ) + ( words * BytesPerWord ) );
}

Object* Object::wordsDown( void* pointer, intptr_t words )
{
	return wordsUp( pointer, -words );
}

/* ---------- ByteObject ---------- */

uint32_t ByteObject::size() const
{
	return ObjectHeader::size( header );
}

void ByteObject::setSize( uint32_t size )
{
	header = ObjectHeader::withSize( size );
}

bool ByteObject::isBinary() const
{
	return ( header & ObjectHeader::binaryFlag ) != 0;
}

void ByteObject::setBinary()
{
	header |= ObjectHeader::binaryFlag;
}

uint8_t* ByteObject::bytePtr()
{
	return bytes;
}

/* ---------- MObject ---------- */

uint32_t MObject::size() const
{
	return ObjectHeader::size( header );
}

void MObject::setSize( uint32_t size )
{
	header = ObjectHeader::withSize( size );
}

bool MObject::isBinary() const
{
	return ( header & ObjectHeader::binaryFlag ) != 0;
}

void MObject::setBinary()
{
	header |= ObjectHeader::binaryFlag;
}

bool MObject::isGcDone() const
{
	return ( header & ObjectHeader::gcDoneFlag ) != 0;
}

void MObject::setGcDone()
{
	header |= ObjectHeader::gcDoneFlag;
}

/* ---------- SmallInt ---------- */

bool SmallInt::is( const void* i )
{
	return ( ( (intptr_t) i ) & 0x01 ) != 0;
}

bool SmallInt::fits( intptr_t value )
{
	return ( value >= INT_MIN / 2 ) && ( value <= INT_MAX / 2 );
}

int SmallInt::value( const void* i )
{
	return (int) ( ( (intptr_t) i ) >> 1 );
}

Object* SmallInt::make( intptr_t value )
{
	return (Object*) ( ( value * 2 ) | 0x01 );
}

/* ---------- Symbol ---------- */

int Symbol::compare( const uint8_t* left, int leftSize, const uint8_t* right, int rightSize )
{
	int minSize = leftSize < rightSize ? leftSize : rightSize;

	for( int i = 0; i < minSize; i++ )
		if( left[ i ] != right[ i ] )
			return left[ i ] < right[ i ] ? -1 : 1;
	return leftSize - rightSize;
}

int Symbol::compare( Object* left, Object* right )
{
	return compare( left->bytePtr(), (int) left->size(), right->bytePtr(), (int) right->size() );
}

int Symbol::compare( const char* left, Object* right )
{
	return compare( (const uint8_t*) left, (int) strlen( left ), right->bytePtr(), (int) right->size() );
}
