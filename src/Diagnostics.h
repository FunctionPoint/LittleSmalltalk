#pragma once

/*
	Diagnostics
	Debugging output: a back trace of a context chain (stdout), and dumps
	of objects and methods (stderr).
*/

#include <string_view>

class Object;
class ObjectMemory;
class SpecialObjects;

class Diagnostics
{
private:
	const ObjectMemory& memory;
	const SpecialObjects& objects;

public:
	Diagnostics( const ObjectMemory& memory, const SpecialObjects& objects );

	void backTrace( Object* aContext ) const;
	void dumpObject( Object* object, int indent ) const;
	void dumpMethod( Object* aMethod, int indent ) const;

	static void dumpDictKeys( Object* dictionary );

private:
	void printContext( Object* aContext ) const;
	void printArguments( Object* arguments ) const;
	void printArgument( Object* argument ) const;
	static std::string_view textOf( Object* object );
};
