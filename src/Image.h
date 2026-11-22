#pragma once

/*
	Image
	An image file: reads one into object memory and sets the special
	objects (nil, true, false, the globals, the classes the VM uses), or
	writes one: the header, then the object graph from a root object (the
	globals dictionary), depth first. An object that was already written is
	written as a reference to its index; index 0 (the first object written)
	is written as nil. Used by the VM (loading an image, primitive 104) and
	by the CompiledImage (lst -o).
	Only version 6, the version the bootstrapper and the VM write: the older
	formats (a raw memory dump, and images without a header) are not
	supported.
*/

#include <stdint.h>
#include <stdio.h>
#include <unordered_map>

class Object;
class ObjectMemory;
class SpecialObjects;

class Image
{
public:
	static constexpr int imageHeaderSize = 5;
	static constexpr int imageVersion = 6;

	/* these define the flags used for writing and reading images.
	The bytes per word or size is usually stored in the lower bits */
	static constexpr int errorType = 0;
	static constexpr int objectType = 1 << 5;
	static constexpr int positiveIntegerType = 2 << 5;
	static constexpr int negativeIntegerType = 3 << 5;
	static constexpr int byteArrayType = 4 << 5;
	static constexpr int previousObjectType = 5 << 5;
	static constexpr int nilType = 6 << 5;

	static constexpr int smallTagLimit = 0x0F;
	static constexpr int largeTagFlag = 0x10;
	static constexpr int tagSizeMask = 0x1F;
	static constexpr int tagValueMask = smallTagLimit;
	static constexpr int tagTypeMask = 0xE0;

private:
	ObjectMemory& memory;
	SpecialObjects& objects;

	/* reading */
	int indirectionTop = 0;
	Object** indirectionArray = nullptr;
	int byteOffset = 0;

	/* writing */
	std::unordered_map<Object*, int> written;

public:
	Image( ObjectMemory& memory, SpecialObjects& objects );
	Image( const Image& ) = delete;
	Image& operator=( const Image& ) = delete;

	int read( FILE* file );
	int write( FILE* file, Object* root );

private:
	/* reading */
	static uint8_t readVersion( FILE* file );
	int readObjects( FILE* file );
	Object* objectRead( FILE* file );
	void readTag( FILE* file, int* type, int* value );
	int getByte( FILE* file );

	/* writing */
	static void writeVersion( FILE* file, uint8_t version );
	void objectWrite( FILE* file, Object* object );
	void writeTag( FILE* file, int type, int value );
	static int getIntSize( int value );
};
