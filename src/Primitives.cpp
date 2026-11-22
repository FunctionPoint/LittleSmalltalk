#include "platform.h"
#include <string.h>
#include <stdio.h>
#include <iostream>
#include <string_view>

#include "Image.h"
#include "Log.h"
#include "ObjectMemory.h"
#include "Primitives.h"
#include "RuntimeCompiler.h"
#include "SpecialObjects.h"

Primitives::Primitives( ObjectMemory& memory, SpecialObjects& objects )
	: memory( memory ), objects( objects )
{
}

Primitives::~Primitives()
{
	/* the files Smalltalk did not close */
	for( FILE* file : filePointers )
		if( file )
			fclose( file );
}

void Primitives::setArguments( int argumentCount, const char** arguments )
{
	programArgumentCount = argumentCount;
	programArguments = arguments;
}

/*
	primitive handler
	(note that many primitives are handled in the interpreter)
*/

Object* Primitives::call( int primitiveNumber, Object* arguments, bool& failed )
{
	failed = false;
	switch( primitiveNumber ) {
	case 100:
		return openFile( arguments, failed );
	case 101:
		return readCharacter( arguments, failed );
	case 102:
		return writeCharacter( arguments, failed );
	case 103:
		return closeFile( arguments, failed );
	case 104:
		return writeImage( arguments, failed );
	case 106:
		return readBytes( arguments, failed );
	case 107:
		return writeBytes( arguments, failed );
	case 108:
		return seekFile( arguments, failed );
	case 150:
		return findSubstring( arguments, failed );
	case 160:
		return printLog( arguments, failed );
	case 161:
		return timestamp( arguments, failed );
	case 170:
		return commandLineArguments( arguments, failed );
	case 180:
		return compileMethod( arguments, failed );
	case 181:
		return compareSymbols( arguments, failed );
	default:
		Log::error( "unknown primitive %d!", primitiveNumber );
	}
	return objects.nilObject;
}

/* 100: open a file; the result is its number */
Object* Primitives::openFile( Object* arguments, bool& failed )
{
	int pathSize = arguments->data[ 0 ]->size() + 1;
	char* pathBuffer = (char*) alloca( (size_t) pathSize );
	int modeSize = arguments->data[ 1 ]->size() + 1;
	char* modeBuffer = (char*) alloca( (size_t) modeSize + 1 );

	copyToCString( pathBuffer, pathSize, arguments->data[ 0 ] );
	copyToCString( modeBuffer, modeSize, arguments->data[ 1 ] );

	/* Images are binary, and Smalltalk sources use LF: never let Windows translate line ends. */
	if( !strchr( modeBuffer, 'b' ) )
		strcat( modeBuffer, "b" );

	FILE* file = fopen( pathBuffer, modeBuffer );
	if( file == NULL ) {
		failed = true;
		return objects.nilObject;
	}
	/* its number: the first free one (of a closed file), else a new one */
	size_t i = 0;
	while( i < filePointers.size() && filePointers[ i ] )
		i++;
	if( i == filePointers.size() )
		filePointers.push_back( file );
	else
		filePointers[ i ] = file;
	return SmallInt::make( (intptr_t) i );
}

void Primitives::copyToCString( char* to, int size, Object* from )
{
	int fromSize = from->size();
	ByteObject* byteObject = (ByteObject*) from;

	if( fromSize >= size )
		Log::error( "String too long, %d, for buffer, %d bytes!", fromSize, size );

	for( int i = 0; i < fromSize; i++ )
		to[ i ] = (char) byteObject->bytes[ i ];

	to[ fromSize ] = '\0'; /* put null terminator at end */
}

/* 101: read a single character from a file; nil at the end of the file */
Object* Primitives::readCharacter( Object* arguments, bool& failed )
{
	FILE* file = fileAt( arguments->data[ 0 ] );

	if( !file ) {
		failed = true;
		return objects.nilObject;
	}
	int inputChar = fgetc( file );
	if( inputChar == EOF )
		return objects.nilObject;
	return SmallInt::make( inputChar );
}

/* the open file with the number fileNumber (a SmallInt), or nullptr */
FILE* Primitives::fileAt( Object* fileNumber ) const
{
	if( !SmallInt::is( fileNumber ) )
		return nullptr;
	int i = SmallInt::value( fileNumber );
	if( ( i < 0 ) || ( i >= (int) filePointers.size() ) )
		return nullptr;
	return filePointers[ i ];
}

/* 102: write a single character to a file */
Object* Primitives::writeCharacter( Object* arguments, bool& failed )
{
	FILE* file = fileAt( arguments->data[ 0 ] );

	if( !file ) {
		failed = true;
		return objects.nilObject;
	}
	fputc( SmallInt::value( arguments->data[ 1 ] ), file );
	return objects.nilObject;
}

/* 103: close a file */
Object* Primitives::closeFile( Object* arguments, bool& failed )
{
	FILE* file = fileAt( arguments->data[ 0 ] );

	if( !file ) {
		failed = true;
		return objects.nilObject;
	}
	fclose( file );
	/* the number is free again: it no longer refers to the closed file */
	filePointers[ SmallInt::value( arguments->data[ 0 ] ) ] = nullptr;
	return objects.nilObject;
}

/* 104: write the image to a file */
Object* Primitives::writeImage( Object* arguments, bool& failed )
{
	FILE* file = fileAt( arguments->data[ 0 ] );

	if( !file ) {
		failed = true;
		return objects.nilObject;
	}
	Image( memory, objects ).write( file, objects.globalsObject );
	return objects.nilObject;
}

/* 106: read from a file into a ByteArray; the result is the number of bytes read */
Object* Primitives::readBytes( Object* arguments, bool& failed )
{
	/* File descriptor */
	FILE* file = fileAt( arguments->data[ 0 ] );
	if( !file ) {
		failed = true;
		return objects.nilObject;
	}

	/* Make sure we're populating an array of bytes */
	Object* bytes = arguments->data[ 1 ];
	if( !bytes->isBinary() ) {
		failed = true;
		return objects.nilObject;
	}

	/* Sanity check on I/O count */
	int byteCount = SmallInt::value( arguments->data[ 2 ] );
	if( ( byteCount < 0 ) || ( byteCount > (int) bytes->size() ) ) {
		failed = true;
		return objects.nilObject;
	}

	/* Do the I/O */
	byteCount = (int) fread( bytes->bytePtr(), sizeof( char ), (size_t) byteCount, file );
	if( byteCount < 0 ) {
		failed = true;
		return objects.nilObject;
	}
	return SmallInt::make( byteCount );
}

/* 107: write a ByteArray to a file; the result is the number of bytes written */
Object* Primitives::writeBytes( Object* arguments, bool& failed )
{
	/* File descriptor */
	FILE* file = fileAt( arguments->data[ 0 ] );
	if( !file ) {
		failed = true;
		return objects.nilObject;
	}

	/* Make sure we're writing an array of bytes */
	Object* bytes = arguments->data[ 1 ];
	if( !bytes->isBinary() ) {
		failed = true;
		return objects.nilObject;
	}

	/* Sanity check on I/O count */
	int byteCount = SmallInt::value( arguments->data[ 2 ] );
	if( ( byteCount < 0 ) || ( byteCount > (int) bytes->size() ) ) {
		failed = true;
		return objects.nilObject;
	}

	/* Do the I/O */
	byteCount = (int) fwrite( bytes->bytePtr(), sizeof( char ), (size_t) byteCount, file );
	if( byteCount < 0 ) {
		failed = true;
		return objects.nilObject;
	}
	return SmallInt::make( byteCount );
}

/* 108: seek to a file position */
Object* Primitives::seekFile( Object* arguments, bool& failed )
{
	/* File descriptor */
	FILE* file = fileAt( arguments->data[ 0 ] );
	if( !file ) {
		failed = true;
		return objects.nilObject;
	}

	/* File position */
	int position = SmallInt::value( arguments->data[ 1 ] );
	int returnCode;
	if( ( position < 0 ) || ( ( returnCode = fseek( file, position, SEEK_SET ) ) < 0 ) ) {
		failed = true;
		return objects.nilObject;
	}

	/* Return position as our value (this returns fseek's result, 0) */
	return SmallInt::make( returnCode );
}

/* 150: match a substring in a string: its index, or nil */
Object* Primitives::findSubstring( Object* arguments, bool& failed )
{
	/* make sure we've got strings */
	if( !arguments->data[ 0 ]->isBinary() ) {
		std::cout << "#position: failed, first arg is not a binary object.\n";
		failed = true;
		return objects.nilObject;
	}

	if( !arguments->data[ 1 ]->isBinary() ) {
		std::cout << "#position: failed, second arg is not a binary object.\n";
		failed = true;
		return objects.nilObject;
	}

	/* get the sizes of the strings */
	int stringSize = arguments->data[ 0 ]->size();
	int substringSize = arguments->data[ 1 ]->size();

	/*
	 * don't bother to compare if either string has a zero length
	 * or the second string is longer than the first
	 */

	if( ( stringSize > 0 ) && ( substringSize > 0 ) && ( stringSize >= substringSize ) ) {
		/* using alloca to make sure that the memory is freed automatically */
		char* string = (char*) alloca( (size_t) stringSize + 1 );
		char* substring = (char*) alloca( (size_t) substringSize + 1 );
		char* found = (char*) 0;

		copyToCString( string, stringSize + 1, arguments->data[ 0 ] );
		copyToCString( substring, substringSize + 1, arguments->data[ 1 ] );

		/* find the pointer to the substring */
		found = strstr( string, substring );

		if( found != NULL )
			/* the string was found */
			return SmallInt::make( ( found - string ) + 1 );
		return objects.nilObject;
	}
	if( ( stringSize < 0 ) || ( substringSize < 0 ) ) {
		std::cout << "#position: failed due to unusable string sizes, string 1 size " << stringSize << ", string 2 size " << substringSize << ".\n";
		failed = true;
		return objects.nilObject;
	}

	/* a size is 0, or the substring is longer: no match, but no error. */
	return objects.nilObject;
}

/* 160: print a message string */
Object* Primitives::printLog( Object* arguments, bool& failed )
{
	ByteObject* message = (ByteObject*) ( arguments->data[ 0 ] );

	std::cout << "Log: " << std::string_view( (const char*) message->bytePtr(), message->size() ) << '\n';

	failed = false;
	return objects.nilObject;
}

/* 161: an Integer with the microsecond timestamp */
Object* Primitives::timestamp( Object* arguments, bool& failed )
{
	failed = false;
	return newLInteger( lstTimeUsec() );
}

/* 170: the command line arguments as an Array of Strings */
Object* Primitives::commandLineArguments( Object* arguments, bool& failed )
{
	Object* argumentArray = NULL;

	/* allocate enough space for the result Array. */
	argumentArray = memory.allocate( programArgumentCount );
	argumentArray->objectClass = objects.ArrayClass;

	/* we are going to allocate Strings and that could cause GC. */
	memory.pushRoot( argumentArray );

	for( int argumentIndex = 0; argumentIndex < programArgumentCount; argumentIndex++ ) {
		Object* argumentString = NULL;
		const char* argument = programArguments[ argumentIndex ];
		int argumentLength = (int) strlen( argument );

		/* could cause GC */
		argumentString = memory.allocateBytes( argumentLength );
		argumentString->objectClass = objects.StringClass;

		/* copy the bytes. */
		for( int byteIndex = 0; byteIndex < argumentLength; byteIndex++ )
			argumentString->bytePtr()[ byteIndex ] = (uint8_t) argument[ byteIndex ];

		/* get the pointer to the array again.   Could have changed due to GC. */
		argumentArray = memory.peekRoot();

		argumentArray->data[ argumentIndex ] = argumentString;
	}

	argumentArray = memory.popRoot();

	failed = false;
	return argumentArray;
}

/* 180: compile a method: the source text and the instance variable names (see RuntimeCompiler) */
Object* Primitives::compileMethod( Object* arguments, bool& failed )
{
	if( arguments->size() != 2 ) {
		failed = true;
		return objects.nilObject;
	}
	RuntimeCompiler compiler( memory, objects, *this );
	return compiler.compile( arguments->data[ 0 ], arguments->data[ 1 ], failed );
}

/* 181: compare a Symbol with a Symbol or String by their bytes (Symbol::compare): -1, 0 or 1 */
Object* Primitives::compareSymbols( Object* arguments, bool& failed )
{
	Object* left = arguments->data[ 0 ];
	Object* right = arguments->data[ 1 ];
	auto isText = [ this ]( Object* object ) { return !SmallInt::is( object ) && object->isBinary() && ( object->objectClass == objects.SymbolClass || object->objectClass == objects.StringClass ); };

	if( arguments->size() != 2 || !isText( left ) || !isText( right ) ) {
		failed = true;
		return objects.nilObject;
	}
	int order = Symbol::compare( left, right );
	return SmallInt::make( order < 0 ? -1 : ( order > 0 ? 1 : 0 ) );
}

/*
 * newLInteger()
 *  Create new Integer (64-bit)
 */
Object* Primitives::newLInteger( int64_t value )
{
	Object* result = memory.allocateBytes( sizeof( int64_t ) );
	result->objectClass = objects.IntegerClass;
	int64_t* integerPointer = (int64_t*) result->bytePtr();
	*integerPointer = value;
	return ( result );
}

/*
 * do_Integer()
 *  Implement the appropriate 64-bit Integer operation
 *
 * Returns NULL on error, otherwise the resulting Integer or
 * Boolean (for comparisons) object.
 */
Object* Primitives::doInteger( int operation, Object* low, Object* high )
{
	int64_t* integerPointer = (int64_t*) low->bytePtr();
	int64_t lowValue = *integerPointer;
	integerPointer = (int64_t*) high->bytePtr();
	int64_t highValue = *integerPointer;
	switch( operation ) {
	case 25: /* Integer division */
		if( highValue == 0LL )
			return ( NULL );
		return ( newLInteger( lowValue / highValue ) );

	case 26: /* Integer remainder */
		if( highValue == 0LL )
			return ( NULL );
		return ( newLInteger( lowValue % highValue ) );

	case 27: /* Integer addition */
		return ( newLInteger( lowValue + highValue ) );

	case 28: /* Integer multiplication */
		return ( newLInteger( lowValue * highValue ) );

	case 29: /* Integer subtraction */
		return ( newLInteger( lowValue - highValue ) );

	case 30: /* Integer less than */
		return ( ( lowValue < highValue ) ? objects.trueObject : objects.falseObject );

	case 31: /* Integer equality */
		return ( ( lowValue == highValue ) ? objects.trueObject : objects.falseObject );

	default:
		Log::error( "doInteger(): Invalid primitive integer operation %d!", operation );
	}
	return ( NULL );
}
