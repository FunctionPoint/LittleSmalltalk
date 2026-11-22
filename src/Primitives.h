#pragma once

/*
	Primitives
	The primitives that are not done by the Interpreter itself: files
	(100-108), substring match (150), log and timestamps (160, 161), the
	command line arguments (170), compiling a method (180, the
	RuntimeCompiler), comparing a Symbol by its bytes (181); and the
	64-bit Integer objects.
*/

#include <stdint.h>
#include <stdio.h>
#include <vector>

class Object;
class ObjectMemory;
class SpecialObjects;

class Primitives
{
private:
	ObjectMemory& memory;
	SpecialObjects& objects;

	std::vector<FILE*> filePointers;
	int programArgumentCount = 0;
	const char** programArguments = nullptr;

public:
	Primitives( ObjectMemory& memory, SpecialObjects& objects );
	~Primitives();
	Primitives( const Primitives& ) = delete;
	Primitives& operator=( const Primitives& ) = delete;

	void setArguments( int argumentCount, const char** arguments );
	Object* call( int primitiveNumber, Object* arguments, bool& failed );

	Object* newLInteger( int64_t value );
	Object* doInteger( int operation, Object* low, Object* high );

private:
	Object* openFile( Object* arguments, bool& failed );
	static void copyToCString( char* to, int size, Object* from );
	Object* readCharacter( Object* arguments, bool& failed );
	FILE* fileAt( Object* fileNumber ) const;
	Object* writeCharacter( Object* arguments, bool& failed );
	Object* closeFile( Object* arguments, bool& failed );
	Object* writeImage( Object* arguments, bool& failed );
	Object* readBytes( Object* arguments, bool& failed );
	Object* writeBytes( Object* arguments, bool& failed );
	Object* seekFile( Object* arguments, bool& failed );
	Object* findSubstring( Object* arguments, bool& failed );
	Object* printLog( Object* arguments, bool& failed );
	Object* timestamp( Object* arguments, bool& failed );
	Object* commandLineArguments( Object* arguments, bool& failed );
	Object* compileMethod( Object* arguments, bool& failed );
	Object* compareSymbols( Object* arguments, bool& failed );
};
