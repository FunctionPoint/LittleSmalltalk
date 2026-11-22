#include <stdint.h>
#include <stdio.h>

#include "Image.h"
#include "Log.h"
#include "Object.h"
#include "ObjectMemory.h"
#include "SpecialObjects.h"

Image::Image( ObjectMemory& memory, SpecialObjects& objects )
	: memory( memory ), objects( objects )
{
}

int Image::read( FILE* file )
{
	uint8_t version = readVersion( file );

	if( version != imageVersion )
		Log::error( "Unsupported image file version: %u (only version %d images can be read).", version, imageVersion );

	return readObjects( file );
}

uint8_t Image::readVersion( FILE* file )
{
	uint8_t header[ imageHeaderSize ];

	int returnCode = (int) fread( &header, sizeof( header ), 1, file );

	if( returnCode != 1 )
		Log::error( "readVersion(): Unable to read version header data!" );

	if( header[ 0 ] == 'l' && header[ 1 ] == 's' && header[ 2 ] == 't' && header[ 3 ] == '!' )
		return header[ 4 ];

	/* not an image with a header */
	return 0;
}

int Image::readObjects( FILE* file )
{
	Object* specialSymbols = NULL;

	/* use the currently unused space for the indir pointers */
	indirectionArray = (Object**) memory.unusedSpace();
	indirectionTop = 0;

	/* read the base objects from the image file. */

	objects.globalsObject = objectRead( file );
	memory.addStaticRoot( &objects.globalsObject );

	/* fix up everything from globals. */
	specialSymbols = objects.lookupGlobal( "specialSymbols" );

	objects.binaryMessages[ 0 ] = specialSymbols->data[ 0 ];
	memory.addStaticRoot( &objects.binaryMessages[ 0 ] );

	objects.binaryMessages[ 1 ] = specialSymbols->data[ 1 ];
	memory.addStaticRoot( &objects.binaryMessages[ 1 ] );

	objects.binaryMessages[ 2 ] = specialSymbols->data[ 2 ];
	memory.addStaticRoot( &objects.binaryMessages[ 2 ] );

	objects.badMethodSymbol = specialSymbols->data[ 3 ];
	memory.addStaticRoot( &objects.badMethodSymbol );

	/* look up the rest in the globals object. */
	objects.findInGlobals( memory );

	return indirectionTop;
}

Object* Image::objectRead( FILE* file )
{
	Object* newObject = (Object*) 0;

	/* get the tag header for the object, this has a type and value */
	int type;
	int value;
	readTag( file, &type, &value );

	switch( type ) {
	case errorType: /* nil object */
		Log::error( "objectRead(): Read in a NULL object!" );

		break;

	case objectType: /* ordinary object */
	{
		int size = value;
		newObject = memory.allocate( size );
		indirectionArray[ indirectionTop++ ] = newObject;
		newObject->objectClass = objectRead( file );

		/* get object field values. */
		for( int i = 0; i < size; i++ )
			newObject->data[ i ] = objectRead( file );

		break;
	}

	case positiveIntegerType: /* positive integer */
		newObject = SmallInt::make( value );
		break;

	case negativeIntegerType: /* negative integer */
		newObject = SmallInt::make( -value );
		break;

	case byteArrayType: /* byte arrays */
	{
		int size = value;
		//        newObject = staticIAllocate(size);
		newObject = memory.allocateBytes( size );
		indirectionArray[ indirectionTop++ ] = newObject;
		ByteObject* newByteObject = (ByteObject*) newObject;
		for( int i = 0; i < size; i++ ) {
			/* FIXME check for EOF! */
			newByteObject->bytes[ i ] = (uint8_t) getByte( file );
		}

		newByteObject->objectClass = objectRead( file );
		break;
	}

	case previousObjectType: /* previous object */
		if( value > indirectionTop )
			Log::error( "Illegal previous object index %d (max %d)!", value, indirectionTop );

		newObject = indirectionArray[ value ];

		break;

	case nilType: /* object 0 (nil object) */
		newObject = indirectionArray[ 0 ];
		break;

	default:
		Log::error( "Illegal tag type %d!", type );
		break;
	}

	return newObject;
}

void Image::readTag( FILE* file, int* type, int* value )
{
	int inByte = getByte( file );
	if( inByte == EOF )
		Log::error( "Unexpected EOF reading image file: reading tag byte." );

	int valueSize = (int) ( inByte & tagSizeMask );
	*type = (int) ( inByte & tagTypeMask );

	if( valueSize & largeTagFlag ) {
		uint32_t bytesValue = 0;

		/* large size, actual value is in succeeding
		bytes (tempSize bytes). The value is not sign
		extended. */
		*value = 0;

		/* get the number of bytes in the size field */
		valueSize = valueSize & smallTagLimit;

		if( valueSize > BytesPerWord )
			Log::error( "readTag(): Error reading image file: tag value field exceeds machine word size.  Image created on another machine? Field size=%d", valueSize );

		for( int i = 0; i < valueSize; i++ ) {
			inByte = getByte( file );

			if( inByte == EOF )
				Log::error( "readTag(): Unexpected EOF reading image file!" );

			bytesValue = bytesValue | ( ( (uint32_t) inByte & (uint32_t) 0xFF ) << ( (uint32_t) 8 * (uint32_t) i ) );
		}

		*value = (int) (unsigned int) bytesValue;
	} else
		*value = valueSize;
}

int Image::getByte( FILE* file )
{
	int inByte = fgetc( file );
	byteOffset++;

	// fprintf(stderr, "%06d: %u (%x)\n", offset, inByte, inByte);

	return inByte;
}

int Image::write( FILE* file, Object* root )
{
	written.clear();

	/* write the header. */
	writeVersion( file, imageVersion );

	/* write the main objects. */
	objectWrite( file, root );

	return (int) written.size();
}

void Image::writeVersion( FILE* file, uint8_t version )
{
	uint8_t header[ imageHeaderSize ];
	int returnCode = 0;

	header[ 0 ] = (uint8_t) 'l';
	header[ 1 ] = (uint8_t) 's';
	header[ 2 ] = (uint8_t) 't';
	header[ 3 ] = (uint8_t) '!';
	header[ 4 ] = version;

	returnCode = (int) fwrite( &header, sizeof( header ), 1, file );

	if( returnCode != 1 )
		Log::error( "writeVersion(): Unable to write version header data!" );
}

void Image::objectWrite( FILE* file, Object* object )
{
	/* check for illegal object */
	if( object == NULL )
		Log::error( "objectWrite(): writing out a NULL object!" );

	/* small integer?, if so, treat this specially as this is not a pointer */

	if( SmallInt::is( object ) ) { /* SmallInt */
		int integerValue = SmallInt::value( object );

		/* if it is negative, we use the positive value and use a special tag. */
		if( integerValue < 0 )
			writeTag( file, negativeIntegerType, -integerValue );
		else
			writeTag( file, positiveIntegerType, integerValue );
		return;
	}

	/* see if already written: then write its index */
	auto found = written.find( object );
	if( found != written.end() ) {
		if( found->second == 0 )
			writeTag( file, nilType, 0 );
		else
			writeTag( file, previousObjectType, found->second );
		return;
	}

	/* not written, do it now */
	int objectIndex = (int) written.size();
	written.emplace( object, objectIndex );

	int size;
	/* byte objects */
	if( object->isBinary() ) {
		ByteObject* byteObject = (ByteObject*) object;

		size = (int) object->size();

		/* write the header tag */
		writeTag( file, byteArrayType, size );

		/*write out bytes */
		for( int i = 0; i < size; i++ )
			fputc( byteObject->bytes[ i ], file );

		objectWrite( file, object->objectClass );

		return;
	}

	/* ordinary objects */
	size = object->size();

	writeTag( file, objectType, size );

	/* write the class first */
	objectWrite( file, object->objectClass );

	/* write the instance variables of the object */
	for( int i = 0; i < size; i++ )
		objectWrite( file, object->data[ i ] );
}

void Image::writeTag( FILE* file, int type, int value )
{
	/* get the number of bytes required to store the value */
	int valueSize = getIntSize( value );

	if( valueSize ) {
		/*write the tag byte */
		fputc( ( type | valueSize | largeTagFlag ), file );

		for( int i = 0; i < valueSize; i++ )
			fputc( ( value >> ( 8 * i ) ), file );
	} else
		fputc( ( type | value ), file );
}

/* return the size in bytes necessary to accurately handle the integer
value passed.  Note that negatives will always get BytesPerWord size.
This will return zero if the passed value is less than smallTagLimit.
In this case, the value can be packed into the tag it self when read or
written. */
int Image::getIntSize( int value )
{
	/* negatives need sign extension.  this is a to do. */
	if( value < 0 )
		Log::error( "Writing negative value to image without using negative int tag type!" );

	if( value <= smallTagLimit )
		return 0;
	else if( value <= 127 )
		return 1;
	else if( value <= 32767 )
		return 2;
	else if( value <= 8388608 )
		return 3;
	else if( value <= 2147483647 )
		return 4;
	else
		Log::error( "Unusually large value %d requires 8-byte values!", value );

	return BytesPerWord;
}
