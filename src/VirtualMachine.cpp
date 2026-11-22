/*
	Little Smalltalk main program, unix version
	Written by Tim Budd, budd@cs.orst.edu
	All rights reserved, no guarantees given whatsoever.
	May be freely redistributed if not for profit.
*/
#include <stdio.h>
#include <iostream>
#include <string_view>

#include "Compiler.h"
#include "CompiledImage.h"
#include "Image.h"
#include "Log.h"
#include "ObjectLayout.h"
#include "OpCodes.h"
#include "VirtualMachine.h"

VirtualMachine::VirtualMachine()
	: memory(), diagnostics( memory, objects ), primitives( memory, objects ), interpreter( memory, objects, primitives, diagnostics )
{
	/* the method cache holds object pointers, which a collection moves */
	memory.setAfterCollect( [ this ]() { interpreter.flushCache(); } );
}

int VirtualMachine::buildImage( const std::vector<std::string>& sourceFolders, const char* imageFileName )
{
	int objectCount;

	/* a scope of their own: image and compiler keep raw pointers into the static space, so they must be gone before releaseStatic empties it */
	{
		CompiledImage image( memory );
		Compiler compiler( image );

		compiler.compileFolders( sourceFolders );
		objectCount = image.writeImage( imageFileName );
	}

	/* the image is in the file now: nothing points to the objects any more */
	memory.releaseStatic();

	return objectCount;
}

void VirtualMachine::setArguments( int argumentCount, const char** arguments )
{
	primitives.setArguments( argumentCount, arguments );
}

void VirtualMachine::loadImage( const char* imageFileName )
{
	/* read in the method from the image file */
	FILE* file = fopen( imageFileName, "rb" );
	if( !file )
		Log::error( "cannot open image file: %s!", imageFileName );

	/* the reader must not collect garbage (its objects are not reachable yet), so make room
	first: every object and every field is at least one byte in the file and at most 16
	bytes (an object header) in memory */
	fseek( file, 0, SEEK_END );
	long fileSize = ftell( file );
	fseek( file, 0, SEEK_SET );
	if( fileSize > 0 )
		memory.reserve( (size_t) fileSize * 16 );

	Image( memory, objects ).read( file );
	fclose( file );

	/* find the initial method. */
	findInitialMethod();
	memory.addStaticRoot( &objects.initialMethod );
}

void VirtualMachine::findInitialMethod()
{
	/* a program: class Main with a #start method */
	Object* startupClass = objects.lookupGlobal( "Main" );
	if( startupClass ) {
		objects.initialMethod = findMethod( startupClass, "start" );
		if( objects.initialMethod ) {
			startMain = true;
			return;
		}
	}

	/* else the REPL: Shell>>start */
	startupClass = objects.lookupGlobal( "Shell" );
	if( startupClass ) {
		objects.initialMethod = SpecialObjects::dictLookup( startupClass->data[ ObjectLayout::methodsInClass ], "start" );

		if( !objects.initialMethod )
			Log::error( "Unable to find method #start in class Shell!" );

		return;
	}

	Log::error( "Unable to find start up class!" );
}

Object* VirtualMachine::findMethod( Object* _class, const char* selector ) const
{
	for( ; _class && _class != objects.nilObject; _class = _class->data[ ObjectLayout::parentClassInClass ] ) {
		Object* method = SpecialObjects::dictLookup( _class->data[ ObjectLayout::methodsInClass ], selector );
		if( method )
			return method;
	}
	return nullptr;
}

int VirtualMachine::run()
{
	/* build the root process with a context, make sure it is GC-proof */

	rootProcess = memory.allocate( ObjectLayout::processSize );
	rootProcess->objectClass = objects.lookupGlobal( "Process" );
	for( int i = 0; i < ObjectLayout::processSize; i++ )
		rootProcess->data[ i ] = objects.nilObject;
	memory.addStaticRoot( &rootProcess );

	/* context should be dynamic */
	Object* aContext = memory.allocate( ObjectLayout::contextSize );
	aContext->objectClass = objects.ContextClass;
	for( int i = 0; i < ObjectLayout::contextSize; i++ )
		aContext->data[ i ] = objects.nilObject;

	/* add the context to the process. */
	rootProcess->data[ ObjectLayout::contextInProcess ] = aContext;

	/* set up context stack */
	int size = SmallInt::value( objects.initialMethod->data[ ObjectLayout::stackSizeInMethod ] );
	Object* stack = memory.allocate( size );
	stack->objectClass = objects.ArrayClass;
	for( int i = 0; i < size; i++ )
		stack->data[ i ] = objects.nilObject;
	/* the allocation may have moved the context: only the process (a static root) points to it */
	aContext = rootProcess->data[ ObjectLayout::contextInProcess ];
	aContext->data[ ObjectLayout::stackInContext ] = stack;

	/* set up arguments: none for Shell>>start; for Main>>start the receiver, a new Main */
	aContext->data[ ObjectLayout::argumentsInContext ] = objects.nilObject;
	if( startMain ) {
		Object* mainClass = objects.lookupGlobal( "Main" );
		int instanceSize = SmallInt::value( mainClass->data[ ObjectLayout::instanceSizeInClass ] );

		Object* receiver = memory.allocate( instanceSize );
		receiver->objectClass = objects.lookupGlobal( "Main" );
		for( int i = 0; i < instanceSize; i++ )
			receiver->data[ i ] = objects.nilObject;
		memory.pushRoot( receiver );
		Object* arguments = memory.allocate( 1 );
		arguments->objectClass = objects.ArrayClass;
		arguments->data[ 0 ] = memory.popRoot();

		/* the allocations may have moved the context */
		aContext = rootProcess->data[ ObjectLayout::contextInProcess ];
		aContext->data[ ObjectLayout::argumentsInContext ] = arguments;
	}

	/* set up temporary array. */
	size = 19; /* where is this number from? */
	Object* temporaries = memory.allocate( size );
	temporaries->objectClass = objects.ArrayClass;
	for( int i = 0; i < size; i++ )
		temporaries->data[ i ] = objects.nilObject;
	/* the allocation may have moved the context */
	aContext = rootProcess->data[ ObjectLayout::contextInProcess ];
	aContext->data[ ObjectLayout::temporariesInContext ] = temporaries;

	aContext->data[ ObjectLayout::bytePointerInContext ] = SmallInt::make( 0 );
	aContext->data[ ObjectLayout::stackTopInContext ] = SmallInt::make( 0 );
	aContext->data[ ObjectLayout::previousContextInContext ] = objects.nilObject;
	aContext->data[ ObjectLayout::methodInContext ] = objects.initialMethod;

	/* now go do it */
	memory.pushRoot( rootProcess );

	switch( interpreter.execute( rootProcess, 0 ) ) {
	case OpCodes::returnError:
		std::cout << "User defined return\n";
		break;

	case OpCodes::returnBadMethod: {
		std::cout << "can't find method in call\n";
		rootProcess = memory.popRoot();
		Object* selector = rootProcess->data[ ObjectLayout::resultInProcess ];
		std::cout << "Unknown method: " << std::string_view( (const char*) selector->bytePtr(), selector->size() ) << '\n';
		aContext = rootProcess->data[ ObjectLayout::contextInProcess ];
		diagnostics.backTrace( aContext );
		break;
	}

	case OpCodes::returnReturned: /* the startup method returned: nothing to say */
		break;

	case OpCodes::returnTimeExpired:
		std::cout << "time out\n";
		break;

	default:
		std::cout << "unknown return code\n";
		break;
	}

	return ( 0 );
}
