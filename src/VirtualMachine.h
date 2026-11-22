#pragma once

/*
	VirtualMachine
	The Little Smalltalk VM: owns the object memory, the special objects,
	the primitives and the interpreter, and connects them. Can compile an
	image from Smalltalk source folders first (Compiler), loads an image,
	then runs the startup method in a new process: Main>>start on a new
	Main if the image has a class Main that understands #start (the
	program; lst exits when it returns), else Shell>>start (the REPL).
*/

#include <string>
#include <vector>

#include "Diagnostics.h"
#include "Interpreter.h"
#include "ObjectMemory.h"
#include "Primitives.h"
#include "SpecialObjects.h"

class VirtualMachine
{
private:
	/* declared in the order they are made: each one only uses the ones above it */
	ObjectMemory memory;
	SpecialObjects objects;
	Diagnostics diagnostics;
	Primitives primitives;
	Interpreter interpreter;

	bool startMain = false;
	Object* rootProcess = nullptr;

public:
	VirtualMachine();
	VirtualMachine( const VirtualMachine& ) = delete;
	VirtualMachine& operator=( const VirtualMachine& ) = delete;

	void setArguments( int argumentCount, const char** arguments );

	int buildImage( const std::vector<std::string>& sourceFolders, const char* imageFileName );

	void loadImage( const char* imageFileName );

	int run();

private:
	void findInitialMethod();
	Object* findMethod( Object* _class, const char* selector ) const;
};
