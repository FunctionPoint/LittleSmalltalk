/*
	Little Smalltalk main program
	Written by Tim Budd, budd@cs.orst.edu
	All rights reserved, no guarantees given whatsoever.
	May be freely redistributed if not for profit.

	starting point of the little smalltalk system: reads the command line.
	With -o it compiles the Smalltalk sources in source folders into an
	image file, and exits. Else it loads the image into a VirtualMachine and
	runs it: Main>>start if the image has a class Main with a #start method,
	else the REPL, which reads its commands from standard input.
*/
#include <string>
#include <vector>

#include "App.h"

int main( int argumentCount, char** arguments )
{
	App app;

	return app.start( std::vector<std::string>( arguments, arguments + argumentCount ) );
}
