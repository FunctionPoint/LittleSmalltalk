/*
	The layout of a Little Smalltalk object in memory
	Written by Tim Budd, Oregon State University, 1994
	Comments to budd@cs.orst.edu
	All rights reserved, no guarantees given whatsoever.
	May be freely redistributed if not for profit.
*/

/*
	The fundamental data type is the object.
	The first field in an object is a size, the low order two
		bits being used to maintain:
			* binary flag, used if data is binary
			* indirection flag, used if object has been relocated
	The next data field (objectClass) is the class
	The following fields are either objects, or character values

	SmallInts are not objects: their "pointer" holds the value (see SmallInt).

	These classes describe memory that the garbage collector moves around,
	so they stay plain data (all fields public, no constructors, no virtual
	functions); the member functions only read and write the header word.
*/

#pragma once

#include <stdint.h>

/* ints must be at least 32-bit in size! */

constexpr int BytesPerWord = (int) sizeof( intptr_t );

/*
 * The header word: the size is the top 30 bits, the bottom two are flags
 */
class ObjectHeader
{
public:
	static constexpr uintptr_t gcDoneFlag = 0x01;
	static constexpr uintptr_t binaryFlag = 0x02;

	static uint32_t size( uintptr_t header );
	static uintptr_t withSize( uint32_t size );
};

/*
 * The objects end in a C99 flexible array member. C++ has no such thing, but
 * MSVC, GCC and Clang all accept it as an extension; silence MSVC's warning.
 */
#ifdef _MSC_VER
#pragma warning( push )
#pragma warning( disable : 4200 )
#endif

class Object
{
public:
	uintptr_t header;
	Object* objectClass;
	Object* data[];

	uint32_t size() const;
	void setSize( uint32_t size );
	bool isBinary() const;
	void setBinary();
	bool isGcDone() const;
	void setGcDone();
	uint8_t* bytePtr();

	static int toWords( int byteCount );
	static Object* wordsUp( void* pointer, intptr_t words );
	static Object* wordsDown( void* pointer, intptr_t words );
};

/*
	byte objects are used to represent strings and symbols
		bytes per word must be correct
*/

class ByteObject
{
public:
	uintptr_t header;
	Object* objectClass;
	uint8_t bytes[];

	uint32_t size() const;
	void setSize( uint32_t size );
	bool isBinary() const;
	void setBinary();
	uint8_t* bytePtr();
};

/*
 * Used for GC: the class pointer is data[ 0 ] here.
 */

class MObject
{
public:
	uintptr_t header;
	MObject* data[];

	uint32_t size() const;
	void setSize( uint32_t size );
	bool isBinary() const;
	void setBinary();
	bool isGcDone() const;
	void setGcDone();
};

#ifdef _MSC_VER
#pragma warning( pop )
#endif

/*
 * SmallInt objects are used to represent short integers.  They are
 * encoded as 31 bits, signed, with the low bit set to 1.  This
 * distinguishes them from all other objects, which are longword
 * aligned and are proper C memory pointers.
 */
class SmallInt
{
public:
	static bool is( const void* i );
	static bool fits( intptr_t value );
	static int value( const void* i );
	static Object* make( intptr_t value );
};

/*
 * Symbols (and other byte objects) compare by their bytes, unsigned, like
 * strcmp: negative, 0 or positive. Used to keep the keys of a dictionary
 * sorted (the bootstrapper's CompiledImage) and to search them (method lookup,
 * SpecialObjects::dictLookup).
 */
class Symbol
{
public:
	static int compare( const uint8_t* left, int leftSize, const uint8_t* right, int rightSize );
	static int compare( Object* left, Object* right );
	static int compare( const char* left, Object* right );
};
