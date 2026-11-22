/*
	Little Smalltalk version 4

	Written by Tim Budd, Oregon State University, July 1994

		budd@cs.orst.edu

	bytecode interpreter module

	given a process object, execute bytecodes in a tight loop.

	performs subroutine calls for
	a) garbage collection
		b) finding a non-cached method
		c) executing a primitive
	d) creating an integer

	otherwise simply loops until time slice has ended

	Modified by Kyle Hayes for 64-bit systems.
*/

#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <iostream>

#include "Diagnostics.h"
#include "Interpreter.h"
#include "Log.h"
#include "ObjectLayout.h"
#include "ObjectMemory.h"
#include "OpCodes.h"
#include "Primitives.h"
#include "SpecialObjects.h"

Interpreter::Interpreter( ObjectMemory& memory, SpecialObjects& objects, Primitives& primitives, const Diagnostics& diagnostics )
	: memory( memory ), objects( objects ), primitives( primitives ), diagnostics( diagnostics )
{
}

int Interpreter::execute( Object* aProcess, int ticks )
{
	/* push process, so as to save it */
	memory.pushRoot( aProcess );

	/* get current context information */
	context = aProcess->data[ ObjectLayout::contextInProcess ];

	method = context->data[ ObjectLayout::methodInContext ];

	/* load byte pointer */
	byteCodes = (uint8_t*) method->data[ ObjectLayout::byteCodesInMethod ]->bytePtr();
	bytePointer = SmallInt::value( context->data[ ObjectLayout::bytePointerInContext ] );

	/* load stack */
	stack = context->data[ ObjectLayout::stackInContext ];
	stackTop = SmallInt::value( context->data[ ObjectLayout::stackTopInContext ] );

	returnedValue = objects.nilObject;

	/* everything else can wait, as maybe won't be needed at all */
	temporaries = instanceVariables = arguments = literals = 0;

	for( ;; ) {
		int status = continueExecution;

		/*
		 * If we're running against a CPU tick count, stop execution
		 * when we expire the given number of ticks.
		 */
		if( ticks && ( --ticks == 0 ) ) {
			aProcess = memory.popRoot();
			aProcess->data[ ObjectLayout::contextInProcess ] = context;
			aProcess->data[ ObjectLayout::resultInProcess ] = returnedValue;
			context->data[ ObjectLayout::bytePointerInContext ] = SmallInt::make( bytePointer );
			context->data[ ObjectLayout::stackTopInContext ] = SmallInt::make( stackTop );
			return ( OpCodes::returnTimeExpired );
		}

		/* Otherwise decode the instruction */
		int high = byteCodes[ bytePointer++ ];
		int low = high & 0x0F;
		high >>= 4;
		if( high == OpCodes::extended ) {
			high = low;
			low = byteCodes[ bytePointer++ ];
		}

		/* And dispatch */
		switch( high ) {
		case OpCodes::pushInstance:
			pushInstance( low );
			break;
		case OpCodes::pushArgument:
			pushArgument( low );
			break;
		case OpCodes::pushTemporary:
			pushTemporary( low );
			break;
		case OpCodes::pushLiteral:
			pushLiteral( low );
			break;
		case OpCodes::pushConstant:
			pushConstant( low );
			break;
		case OpCodes::pushBlock:
			pushBlock( low );
			break;
		case OpCodes::assignInstance:
			assignInstance( low );
			break;
		case OpCodes::assignTemporary:
			assignTemporary( low );
			break;
		case OpCodes::markArguments:
			markArguments( low );
			break;
		case OpCodes::sendMessage:
			sendMessage( low );
			break;
		case OpCodes::sendUnary: /* optimize certain unary messages */
			sendUnary( low );
			break;
		case OpCodes::sendBinary: /* optimize certain binary messages */
			sendBinary( low );
			break;
		case OpCodes::doPrimitive:
			status = doPrimitive( low );
			break;
		case OpCodes::doSpecial:
			status = doSpecial( low );
			break;
		default:
			Log::error( "invalid bytecode %d!", high );
			break;
		}
		if( status != continueExecution )
			return status;
	}
}

void Interpreter::pushInstance( int low )
{
	if( !arguments )
		arguments = context->data[ ObjectLayout::argumentsInContext ];
	if( !instanceVariables )
		instanceVariables =
			arguments->data[ ObjectLayout::receiverInArguments ];
	stack->data[ stackTop++ ] = instanceVariables->data[ low ];
}

void Interpreter::pushArgument( int low )
{
	if( !arguments )
		arguments = context->data[ ObjectLayout::argumentsInContext ];
	stack->data[ stackTop++ ] = arguments->data[ low ];
}

void Interpreter::pushTemporary( int low )
{
	if( !temporaries )
		temporaries = context->data[ ObjectLayout::temporariesInContext ];
	stack->data[ stackTop++ ] = temporaries->data[ low ];
}

void Interpreter::pushLiteral( int low )
{
	if( !literals )
		literals = method->data[ ObjectLayout::literalsInMethod ];
	stack->data[ stackTop++ ] = literals->data[ low ];
}

void Interpreter::pushConstant( int low )
{
	switch( low ) {
	case 0:
	case 1:
	case 2:
	case 3:
	case 4:
	case 5:
	case 6:
	case 7:
	case 8:
	case 9:
		stack->data[ stackTop++ ] = SmallInt::make( low );
		break;
	case OpCodes::nilConstant:
		stack->data[ stackTop++ ] = objects.nilObject;
		break;
	case OpCodes::trueConstant:
		stack->data[ stackTop++ ] = objects.trueObject;
		break;
	case OpCodes::falseConstant:
		stack->data[ stackTop++ ] = objects.falseObject;
		break;
	default:
		Log::error( "unknown push constant %d", low );
	}
}

void Interpreter::pushBlock( int low )
{
	/* create a block object */
	/* low is argument location */
	/* next byte is goto value */
	int high = codeValue();
	bytePointer += codeValueSize;
	memory.pushRoot( context );
	int stackSize = SmallInt::value( method->data[ ObjectLayout::stackSizeInMethod ] );
	Object* object = memory.allocate( stackSize );
	memory.pushRoot( object );
	object->objectClass = objects.ArrayClass;
	memset( object->bytePtr(), 0, (size_t) ( stackSize * BytesPerWord ) );
	returnedValue = memory.allocate( ObjectLayout::blockSize );
	returnedValue->objectClass = objects.BlockClass;
	returnedValue->data[ ObjectLayout::bytePointerInContext ] =
		returnedValue->data[ ObjectLayout::stackTopInBlock ] =
			returnedValue->data[ ObjectLayout::previousContextInBlock ] = NULL;
	returnedValue->data[ ObjectLayout::bytePointerInBlock ] =
		SmallInt::make( bytePointer );
	returnedValue->data[ ObjectLayout::argumentLocationInBlock ] = SmallInt::make( low );
	returnedValue->data[ ObjectLayout::stackInBlock ] = memory.popRoot();
	context = memory.popRoot();
	if( objects.classOf( context ) == objects.BlockClass )
		returnedValue->data[ ObjectLayout::creatingContextInBlock ] =
			context->data[ ObjectLayout::creatingContextInBlock ];
	else
		returnedValue->data[ ObjectLayout::creatingContextInBlock ] = context;
	method = returnedValue->data[ ObjectLayout::methodInBlock ] =
		context->data[ ObjectLayout::methodInBlock ];
	arguments = returnedValue->data[ ObjectLayout::argumentsInBlock ] =
		context->data[ ObjectLayout::argumentsInBlock ];
	temporaries = returnedValue->data[ ObjectLayout::temporariesInBlock ] =
		context->data[ ObjectLayout::temporariesInBlock ];
	stack = context->data[ ObjectLayout::stackInContext ];
	byteCodes = method->data[ ObjectLayout::byteCodesInMethod ]->bytePtr();
	stack->data[ stackTop++ ] = returnedValue;
	/* zero these out just in case GC occurred */
	literals = instanceVariables = 0;
	bytePointer = high;
}

/* a code location (a jump target): the two bytes at bytePointer, low byte first */
int Interpreter::codeValue() const
{
	return byteCodes[ bytePointer ] | ( byteCodes[ bytePointer + 1 ] << 8 );
}

void Interpreter::assignInstance( int low )
{
	if( !arguments )
		arguments = context->data[ ObjectLayout::argumentsInContext ];
	if( !instanceVariables ) {
		instanceVariables =
			arguments->data[ ObjectLayout::receiverInArguments ];
		/* don't pop stack, leave result there */
	}
	instanceVariables->data[ low ] = stack->data[ stackTop - 1 ];

	/*
	 * If changing a static area, need to add to roots
	 */
	if( !memory.isDynamic( instanceVariables ) && memory.isDynamic( stack->data[ stackTop - 1 ] ) )
		memory.addStaticRoot( &instanceVariables->data[ low ] );
}

void Interpreter::assignTemporary( int low )
{
	if( !temporaries )
		temporaries = context->data[ ObjectLayout::temporariesInContext ];
	temporaries->data[ low ] = stack->data[ stackTop - 1 ];
}

void Interpreter::markArguments( int low )
{
	memory.pushRoot( context );
	arguments = memory.allocate( low );
	arguments->objectClass = objects.ArrayClass;
	Object* savedContext = memory.popRoot();
	if( context != savedContext ) { /* gc has occurred */
		/* reload context */
		instanceVariables = temporaries = literals = 0;
		context = savedContext;
		method = context->data[ ObjectLayout::methodInContext ];
		byteCodes = method->data[ ObjectLayout::byteCodesInMethod ]->bytePtr();
		stack = context->data[ ObjectLayout::stackInContext ];
	}
	/* now load new argument array */
	while( low > 0 )
		arguments->data[ --low ] = stack->data[ --stackTop ];
	stack->data[ stackTop++ ] = arguments;
	arguments = 0;
}

void Interpreter::sendMessage( int low )
{
	if( !literals )
		literals = method->data[ ObjectLayout::literalsInMethod ];
	messageSelector = literals->data[ low ];
	arguments = stack->data[ --stackTop ];
	send();
}

void Interpreter::send()
{
	receiverClass = objects.classOf( arguments->data[ ObjectLayout::receiverInArguments ] );
	sendToClass();
}

void Interpreter::sendToClass()
{
	method = cache.find( messageSelector, receiverClass );
	if( !method ) {
		method = lookupMethod( messageSelector, receiverClass );
		if( !method ) {
			if( messageSelector == objects.badMethodSymbol ) {
				diagnostics.backTrace( context );
				Log::error( "doesNotUnderstand: missing" );
			}
			Object* message = memory.allocate( 2 );
			message->objectClass = objects.ArrayClass;
			message->data[ ObjectLayout::receiverInArguments ] = arguments->data[ ObjectLayout::receiverInArguments ];
			message->data[ 1 ] = messageSelector;
			arguments = message;
			messageSelector = objects.badMethodSymbol;
			send();
			return;
		}
		cache.store( messageSelector, receiverClass, method );
	}

	int high;
	/* see if we can optimize tail call */
	if( byteCodes[ bytePointer ] == ( OpCodes::doSpecial * 16 + OpCodes::stackReturn ) )
		high = 1;
	else if( byteCodes[ bytePointer ] == ( OpCodes::doSpecial * 16 + OpCodes::blockReturn ) )
		high = 2;
	else
		high = 0;

	/* build temporaries for new context */
	memory.pushRoot( arguments );
	memory.pushRoot( method );
	memory.pushRoot( context );

	/* get temporary size before allocation might cause GC and move the method pointer. */
	int low = SmallInt::value( method->data[ ObjectLayout::temporarySizeInMethod ] );

	/* create the stack for the context. */
	int stackSize = SmallInt::value( method->data[ ObjectLayout::stackSizeInMethod ] );
	Object* newStack = memory.allocate( stackSize );
	memory.pushRoot( newStack );
	newStack->objectClass = objects.ArrayClass;
	// FIXME - why are the entries not set to nil?
	memset( newStack->bytePtr(), 0, (size_t) ( stackSize * BytesPerWord ) );

	/* create temporary array */
	if( low > 0 ) {
		temporaries = memory.allocate( low );
		temporaries->objectClass = objects.ArrayClass;
		for( int i = 0; i < low; i++ )
			temporaries->data[ i ] = objects.nilObject;
		memory.pushRoot( temporaries ); /* temporaries */
	} else
		// FIXME - why is this not set to nil?
		memory.pushRoot( NULL ); /* why bother */

	/* save where we are */
	context = memory.peekRoot( 2 );
	context->data[ ObjectLayout::stackTopInContext ] = SmallInt::make( stackTop );
	context->data[ ObjectLayout::bytePointerInContext ] = SmallInt::make( bytePointer );

	/* now go off and build the new context */
	context = memory.allocate( ObjectLayout::contextSize );
	context->objectClass = objects.ContextClass;
	temporaries = context->data[ ObjectLayout::temporariesInContext ] = memory.popRoot();
	stack = context->data[ ObjectLayout::stackInContext ] = memory.popRoot();
	stack->objectClass = objects.ArrayClass;
	context->data[ ObjectLayout::stackTopInContext ] = SmallInt::make( 0 );
	stackTop = 0;

	/* where does this context return to? */
	context->data[ ObjectLayout::previousContextInContext ] = memory.popRoot();

	/* maybe we do a little tail call optimization to skip some bytecodes */
	// FIXME - why are we doing this? Is this extra step here really worth it?
	if( high == 1 ) {
		/* optimize out tail call for method return */
		context->data[ ObjectLayout::previousContextInContext ] =
			context->data[ ObjectLayout::previousContextInContext ]->data[ ObjectLayout::previousContextInContext ];
	} else if( high == 2 ) {
		/* optimize out tail call for block return */
		context->data[ ObjectLayout::previousContextInContext ] =
			context->data[ ObjectLayout::previousContextInContext ]->data[ ObjectLayout::creatingContextInBlock ]->data[ ObjectLayout::previousContextInContext ];
	}

	/* stitch up the rest of the new context and get the local execution variables ready. */

	/* save the method and set the local variable */
	method = context->data[ ObjectLayout::methodInContext ] = memory.popRoot();

	/* save the arguments into the new context and set the local variable */
	arguments = context->data[ ObjectLayout::argumentsInContext ] = memory.popRoot();

	/* clear out the local pointers */
	instanceVariables = literals = 0;

	/* the new context will start executing at the first bytecode */
	context->data[ ObjectLayout::bytePointerInContext ] = SmallInt::make( 0 );
	bytePointer = 0;

	/* set up the local bytecode pointer */
	// FIXME - why isn't this using bytePointer()?
	byteCodes = (uint8_t*) ( method->data[ ObjectLayout::byteCodesInMethod ]->data );
}

void Interpreter::sendUnary( int low )
{
	returnedValue = stack->data[ --stackTop ];
	switch( low ) {
	case 0: /* isNil */
		if( returnedValue == objects.nilObject )
			returnedValue = objects.trueObject;
		else
			returnedValue = objects.falseObject;
		break;
	case 1: /* notNil */
		if( returnedValue == objects.nilObject )
			returnedValue = objects.falseObject;
		else
			returnedValue = objects.trueObject;
		break;
	default:
		Log::error( "unimplemented SendUnary %d", low );
	}
	stack->data[ stackTop++ ] = returnedValue;
}

void Interpreter::sendBinary( int low )
{
	if( SmallInt::is( stack->data[ stackTop - 1 ] ) && SmallInt::is( stack->data[ stackTop - 2 ] ) ) {
		int rightValue = SmallInt::value( stack->data[ --stackTop ] );
		int leftValue = SmallInt::value( stack->data[ --stackTop ] );
		/* can only do operations that won't */
		/* trigger garbage collection */
		switch( low ) {
		case 0: /* < */
			if( leftValue < rightValue )
				returnedValue = objects.trueObject;
			else
				returnedValue = objects.falseObject;
			break;
		case 1: /* <= */
			if( leftValue <= rightValue )
				returnedValue = objects.trueObject;
			else
				returnedValue = objects.falseObject;
			break;
		case 2: /* + */
			/* no possibility of garbage col */
			returnedValue = SmallInt::make( leftValue + rightValue );
			break;
		}
		stack->data[ stackTop++ ] = returnedValue;
		return;
	}

	/* not integers, do as message send */
	memory.pushRoot( context );
	arguments = memory.allocate( 2 );
	arguments->objectClass = objects.ArrayClass;
	Object* savedContext = memory.popRoot();
	if( context != savedContext ) { /* gc has occurred */
		/* reload context */
		instanceVariables = temporaries = literals = 0;
		context = savedContext;
		method = context->data[ ObjectLayout::methodInContext ];
		byteCodes = method->data[ ObjectLayout::byteCodesInMethod ]->bytePtr();
		stack = context->data[ ObjectLayout::stackInContext ];
	}

	/* now load new argument array */
	arguments->data[ 1 ] = stack->data[ --stackTop ];
	arguments->data[ 0 ] = stack->data[ --stackTop ];
	/* now go send message */
	messageSelector = objects.binaryMessages[ low ];
	send();
}

int Interpreter::doPrimitive( int argumentCount )
{
	int outcome;

	/* next byte is primitive number */
	int number = byteCodes[ bytePointer++ ];
	memory.pushRoot( context );
	switch( number ) {
	case 1:
		outcome = objectIdentity();
		break;
	case 2:
		outcome = classOfObject();
		break;
	case 3:
		outcome = printCharacter();
		break;
	case 4:
		outcome = objectSize();
		break;
	case 5:
		outcome = arrayAtPut();
		break;
	case 6:
		outcome = executeProcess();
		break;
	case 7:
		outcome = newObject();
		break;
	case 8:
		outcome = invokeBlock( argumentCount );
		break;
	case 9:
		outcome = readCharacter();
		break;
	case 10:
		outcome = smallIntAdd();
		break;
	case 11:
		outcome = smallIntQuotient();
		break;
	case 12:
		outcome = smallIntRemainder();
		break;
	case 13:
		outcome = smallIntLessThan();
		break;
	case 14:
		outcome = smallIntEqual();
		break;
	case 15:
		outcome = smallIntMultiply();
		break;
	case 16:
		outcome = smallIntSubtract();
		break;
	case 19:
		outcome = errorTrap();
		break;
	case 20:
		outcome = newByteArray();
		break;
	case 21:
		outcome = stringAt();
		break;
	case 22:
		outcome = stringAtPut();
		break;
	case 23:
		outcome = stringClone();
		break;
	case 24:
		outcome = arrayAt();
		break;
	case 25: /* Integer division */
	case 26: /* Integer remainder */
	case 27: /* Integer addition */
	case 28: /* Integer multiplication */
	case 29: /* Integer subtraction */
	case 30: /* Integer less than */
	case 31: /* Integer equality */
		outcome = integerOperation( number );
		break;
	case 32:
		outcome = newInteger();
		break;
	case 33:
		outcome = integerLowOrder();
		break;
	case 34:
		outcome = flushMethodCache();
		break;
	case 35:
		outcome = exchangeObjects();
		break;
	case 36:
		outcome = bitOr();
		break;
	case 37:
		outcome = bitAnd();
		break;
	case 38:
		outcome = replaceFrom();
		break;
	case 39:
		outcome = bitShift();
		break;
	case 40:
		outcome = integerTruncated();
		break;
	default:
		outcome = callPrimitive( number, argumentCount );
		break;
	}

	if( outcome == primitiveSucceeded ) {
		/*
		 * Restore our context pointer and
		 * force a stack return due to successful
		 * primitive.
		 */
		context = memory.popRoot();
		return doReturn();
	}
	if( outcome == primitiveFailed )
		return failPrimitive();
	/* the block runs (invokeBlock), or the process ends (errorTrap) */
	return outcome;
}

/* 1: object identity */
int Interpreter::objectIdentity()
{
	returnedValue = stack->data[ --stackTop ];
	if( returnedValue == stack->data[ --stackTop ] )
		returnedValue = objects.trueObject;
	else
		returnedValue = objects.falseObject;
	return primitiveSucceeded;
}

/* 2: object class */
int Interpreter::classOfObject()
{
	returnedValue = stack->data[ --stackTop ];
	returnedValue = objects.classOf( returnedValue );
	return primitiveSucceeded;
}

/* 3: print a single character */
int Interpreter::printCharacter()
{
	int character = SmallInt::value( stack->data[ --stackTop ] );
	std::cout.put( (char) character );
	returnedValue = objects.nilObject;
	return primitiveSucceeded;
}

/* 4: object size */
int Interpreter::objectSize()
{
	returnedValue = stack->data[ --stackTop ];
	int size;
	if( SmallInt::is( returnedValue ) )
		size = 0;
	else
		size = returnedValue->size();
	returnedValue = SmallInt::make( size );
	return primitiveSucceeded;
}

/* 5: Array at put */
int Interpreter::arrayAtPut()
{
	Object* object = stack->data[ --stackTop ];
	if( !SmallInt::is( object ) ) {
		stackTop -= 2;
		return primitiveFailed;
	}
	int i = SmallInt::value( object ) - 1;
	returnedValue = stack->data[ --stackTop ];
	/* Bounds check */
	if( ( i < 0 ) || ( i >= (int) returnedValue->size() ) ) {
		stackTop -= 1;
		return primitiveFailed;
	}

	returnedValue->data[ i ] = stack->data[ --stackTop ];
	/*
	 * If putting a non-static pointer
	 * into an array in static memory,
	 * register it for GC.
	 */
	if( !memory.isDynamic( returnedValue ) && memory.isDynamic( stack->data[ stackTop ] ) )
		memory.addStaticRoot( &returnedValue->data[ i ] );
	return primitiveSucceeded;
}

/* 6: new process execute */
int Interpreter::executeProcess()
{
	int ticks = SmallInt::value( stack->data[ --stackTop ] );
	Object* process = stack->data[ --stackTop ];
	/* the other process runs in these same registers: doPrimitive then loads them again from our context (pushed as a root) */
	int status = execute( process, ticks );

	/* return value as a SmallInt */
	returnedValue = SmallInt::make( status );
	return primitiveSucceeded;
}

/* 7: new object allocation */
int Interpreter::newObject()
{
	int size = SmallInt::value( stack->data[ --stackTop ] );
	memory.pushRoot( stack->data[ --stackTop ] );
	returnedValue = memory.allocate( size );
	returnedValue->objectClass = memory.popRoot();
	while( size > 0 )
		returnedValue->data[ --size ] = objects.nilObject;
	return primitiveSucceeded;
}

/* 8: block invocation; argumentCount includes the block itself */
int Interpreter::invokeBlock( int argumentCount )
{
	returnedValue = stack->data[ --stackTop ];
	/* put arguments in place */
	int argumentLocation = SmallInt::value( returnedValue->data[ ObjectLayout::argumentLocationInBlock ] );
	temporaries = returnedValue->data[ ObjectLayout::temporariesInBlock ];
	int last = argumentCount - 2;
	int freeTemporaries = ( temporaries ? ( (int) temporaries->size() - argumentLocation ) : 0 );
	if( last >= freeTemporaries ) {
		stackTop -= ( last + 1 );
		return primitiveFailed;
	}
	while( last >= 0 ) {
		temporaries->data[ argumentLocation + last ] =
			stack->data[ --stackTop ];
		last--;
	}
	returnedValue->data[ ObjectLayout::previousContextInBlock ] =
		context->data[ ObjectLayout::previousContextInContext ];
	context = returnedValue;
	arguments = instanceVariables =
		literals = 0;
	stack = context->data[ ObjectLayout::stackInContext ];
	stackTop = 0;
	method = context->data[ ObjectLayout::methodInBlock ];
	byteCodes = method->data[ ObjectLayout::byteCodesInMethod ]->bytePtr();
	bytePointer = SmallInt::value( context->data[ ObjectLayout::bytePointerInBlock ] );
	memory.popRoot();
	/* done: execution goes on in the block */
	return continueExecution;
}

/* 9: read char from standard input */
int Interpreter::readCharacter()
{
	int character = getchar();
	if( character == EOF )
		returnedValue = objects.nilObject;
	else
		returnedValue = SmallInt::make( character );
	return primitiveSucceeded;
}

/* 10: small integer addition */
int Interpreter::smallIntAdd()
{
	int high, low;
	if( !popTwoSmallInts( high, low ) )
		return primitiveFailed;
	/* in 64 bits: it cannot overflow (signed overflow is undefined behaviour) */
	returnedValue = smallIntResult( (int64_t) high + low );
	return primitiveSucceeded;
}

/* the result of a SmallInt operation: a SmallInt if it fits, else a 64-bit Integer */
Object* Interpreter::smallIntResult( int64_t result )
{
	if( SmallInt::fits( result ) )
		return SmallInt::make( result );
	return primitives.newLInteger( result );
}

/*
 * Pull two integers of the required class as arguments from stack: low is
 * the top one, high the one below. False if one is not a SmallInt; both are
 * popped anyway.
 */
bool Interpreter::popTwoSmallInts( int& high, int& low )
{
	Object* object = stack->data[ --stackTop ];
	if( !SmallInt::is( object ) ) {
		stackTop -= 1;
		return false;
	}
	low = SmallInt::value( object );
	object = stack->data[ --stackTop ];
	if( !SmallInt::is( object ) )
		return false;
	high = SmallInt::value( object );
	return true;
}

/* 11: small integer division */
int Interpreter::smallIntQuotient()
{
	int high, low;
	if( !popTwoSmallInts( high, low ) )
		return primitiveFailed;
	if( low == 0 )
		return primitiveFailed;
	high /= low;
	returnedValue = SmallInt::make( high );
	return primitiveSucceeded;
}

/* 12: small integer remainder */
int Interpreter::smallIntRemainder()
{
	int high, low;
	if( !popTwoSmallInts( high, low ) )
		return primitiveFailed;
	if( low == 0 )
		return primitiveFailed;
	high %= low;
	returnedValue = SmallInt::make( high );
	return primitiveSucceeded;
}

/* 13: small integer less than */
int Interpreter::smallIntLessThan()
{
	int high, low;
	if( !popTwoSmallInts( high, low ) )
		return primitiveFailed;
	if( high < low )
		returnedValue = objects.trueObject;
	else
		returnedValue = objects.falseObject;
	return primitiveSucceeded;
}

/* 14: small integer equality */
int Interpreter::smallIntEqual()
{
	int high, low;
	if( !popTwoSmallInts( high, low ) )
		return primitiveFailed;
	if( high == low )
		returnedValue = objects.trueObject;
	else
		returnedValue = objects.falseObject;
	return primitiveSucceeded;
}

/* 15: small integer multiplication */
int Interpreter::smallIntMultiply()
{
	int high, low;
	if( !popTwoSmallInts( high, low ) )
		return primitiveFailed;
	/* in 64 bits: two SmallInts (31 bits) multiply to at most 62 bits */
	returnedValue = smallIntResult( (int64_t) high * low );
	return primitiveSucceeded;
}

/* 16: small integer subtraction */
int Interpreter::smallIntSubtract()
{
	int high, low;
	if( !popTwoSmallInts( high, low ) )
		return primitiveFailed;
	returnedValue = smallIntResult( (int64_t) high - low );
	return primitiveSucceeded;
}

/* 19: error trap -- halt execution */
int Interpreter::errorTrap()
{
	memory.popRoot(); /* pop context */
	Object* aProcess = memory.popRoot();
	aProcess->data[ ObjectLayout::contextInProcess ] = context;
	return ( OpCodes::returnError );
}

/* 20: byteArray allocation */
int Interpreter::newByteArray()
{
	int size = SmallInt::value( stack->data[ --stackTop ] );
	memory.pushRoot( stack->data[ --stackTop ] );
	returnedValue = memory.allocateBytes( size );
	returnedValue->objectClass = memory.popRoot();
	memset( returnedValue->bytePtr(), 0, (size_t) size );
	return primitiveSucceeded;
}

/* 21: string at */
int Interpreter::stringAt()
{
	int i = SmallInt::value( stack->data[ --stackTop ] ) - 1;
	returnedValue = stack->data[ --stackTop ];
	if( ( i < 0 ) || ( i >= (int) returnedValue->size() ) )
		return primitiveFailed;
	int character = returnedValue->bytePtr()[ i ];
	returnedValue = SmallInt::make( character );
	return primitiveSucceeded;
}

/* 22: string at put */
int Interpreter::stringAtPut()
{
	int i = SmallInt::value( stack->data[ --stackTop ] ) - 1;
	returnedValue = stack->data[ --stackTop ];
	if( ( i < 0 ) || ( i >= (int) returnedValue->size() ) ) {
		stackTop -= 1;
		return primitiveFailed;
	}
	returnedValue->bytePtr()[ i ] = (uint8_t) (uint32_t) SmallInt::value( stack->data[ --stackTop ] );
	return primitiveSucceeded;
}

/* 23: string clone */
int Interpreter::stringClone()
{
	memory.pushRoot( stack->data[ --stackTop ] );
	returnedValue = stack->data[ --stackTop ];
	memory.pushRoot( returnedValue );
	int size = returnedValue->size();
	returnedValue = memory.allocateBytes( size );
	Object* original = memory.popRoot();
	while( size-- > 0 )
		returnedValue->bytePtr()[ size ] =
			original->bytePtr()[ size ];
	returnedValue->objectClass = memory.popRoot();
	return primitiveSucceeded;
}

/* 24: array at */
int Interpreter::arrayAt()
{
	int i = SmallInt::value( stack->data[ --stackTop ] ) - 1;
	returnedValue = stack->data[ --stackTop ];
	if( ( i < 0 ) || ( i >= (int) returnedValue->size() ) )
		return primitiveFailed;
	returnedValue = returnedValue->data[ i ];
	return primitiveSucceeded;
}

/* 25-31: the 64-bit Integer operation number (Primitives::doInteger) */
int Interpreter::integerOperation( int number )
{
	Object* object = stack->data[ --stackTop ];
	if( objects.classOf( object ) != objects.IntegerClass ) {
		stackTop -= 1;
		return primitiveFailed;
	}
	returnedValue = stack->data[ --stackTop ];
	if( objects.classOf( returnedValue ) != objects.IntegerClass )
		return primitiveFailed;
	returnedValue = primitives.doInteger( number,
										  returnedValue, object );
	if( returnedValue == NULL )
		return primitiveFailed;
	return primitiveSucceeded;
}

/* 32: Integer allocation */
int Interpreter::newInteger()
{
	Object* object = stack->data[ --stackTop ];
	if( !SmallInt::is( object ) )
		return primitiveFailed;
	returnedValue = primitives.newLInteger( SmallInt::value( object ) );
	return primitiveSucceeded;
}

/* 33: Low order of Integer -> SmallInt */
int Interpreter::integerLowOrder()
{
	Object* object = stack->data[ --stackTop ];
	int64_t* integerPointer = (int64_t*) object->bytePtr();
	int64_t largeValue = *integerPointer;
	int result = (int) largeValue;
	if( !SmallInt::fits( result ) )
		return primitiveFailed;
	returnedValue = SmallInt::make( result );
	return primitiveSucceeded;
}

/* 34: Flush method cache */
int Interpreter::flushMethodCache()
{
	cache.flush();
	return primitiveSucceeded;
}

/* 35: Bulk object exchange */
int Interpreter::exchangeObjects()
{
	Object* object = stack->data[ --stackTop ];
	if( object->objectClass != objects.ArrayClass )
		return primitiveFailed;
	returnedValue = stack->data[ --stackTop ];
	if( returnedValue->objectClass != objects.ArrayClass )
		return primitiveFailed;
	if( object->size() != returnedValue->size() )
		return primitiveFailed;
	memory.exchangeObjects( object, returnedValue, object->size() );
	return primitiveSucceeded;
}

/* 36: bitOr: */
int Interpreter::bitOr()
{
	Object* object = stack->data[ --stackTop ];
	if( !SmallInt::is( object ) ) {
		--stackTop;
		return primitiveFailed;
	}
	int high = SmallInt::value( object );
	object = stack->data[ --stackTop ];
	if( !SmallInt::is( object ) )
		return primitiveFailed;
	returnedValue = SmallInt::make( SmallInt::value( object ) | high );
	return primitiveSucceeded;
}

/* 37: bitAnd: */
int Interpreter::bitAnd()
{
	Object* object = stack->data[ --stackTop ];
	if( !SmallInt::is( object ) ) {
		--stackTop;
		return primitiveFailed;
	}
	int high = SmallInt::value( object );
	object = stack->data[ --stackTop ];
	if( !SmallInt::is( object ) )
		return primitiveFailed;
	returnedValue = SmallInt::make( SmallInt::value( object ) & high );
	return primitiveSucceeded;
}

/* 38: replaceFrom:... */
int Interpreter::replaceFrom()
{
	returnedValue = stack->data[ --stackTop ];
	stackTop -= 4;
	if( bulkReplace( returnedValue,
					 stack->data[ stackTop + 3 ],
					 stack->data[ stackTop + 2 ],
					 stack->data[ stackTop + 1 ],
					 stack->data[ stackTop ] ) )
		return primitiveFailed;
	return primitiveSucceeded;
}

/* 39: bitShift: */
int Interpreter::bitShift()
{
	Object* object = stack->data[ --stackTop ];
	if( !SmallInt::is( object ) ) {
		--stackTop;
		return primitiveFailed;
	}
	int low = SmallInt::value( object );
	object = stack->data[ --stackTop ];
	if( !SmallInt::is( object ) )
		return primitiveFailed;
	int high = SmallInt::value( object );
	if( low < 0 )
		/* Negative means shift right */
		low = high >> ( -low );
	else {
		/* Shift left--catch overflow */
		low = high << low;
		if( high > low )
			return primitiveFailed;
	}
	returnedValue = SmallInt::make( low );
	return primitiveSucceeded;
}

/* 40: Truncate Integer -> SmallInt */
int Interpreter::integerTruncated()
{
	Object* object = stack->data[ --stackTop ];
	int64_t* integerPointer = (int64_t*) object->bytePtr();
	int64_t largeValue = *integerPointer;
	int result = (int) largeValue;
	returnedValue = SmallInt::make( result );
	return primitiveSucceeded;
}

/* the other primitives: pop the arguments, and try Primitives::call */
int Interpreter::callPrimitive( int number, int argumentCount )
{
	memory.pushRoot( stack );
	arguments = memory.allocate( argumentCount );
	arguments->objectClass = objects.ArrayClass;
	stack = memory.popRoot();
	for( int i = argumentCount; i > 0; )
		arguments->data[ --i ] =
			stack->data[ --stackTop ];
	bool failed;
	returnedValue = primitives.call( number, arguments, failed );
	if( failed )
		return primitiveFailed;
	arguments = 0;
	return primitiveSucceeded;
}

int Interpreter::failPrimitive()
{
	/*
	 * Since we're continuing execution from a failed
	 * primitive, re-fetch context if a GC had occurred
	 * during the failed execution.  Supply a return value
	 * for the failed primitive.
	 */
	returnedValue = objects.nilObject;
	/* arguments may hold the primitive's argument Array (Primitives::call): fetch
	the method's own arguments again, or self would be the primitive's first argument */
	arguments = 0;
	Object* savedContext = memory.popRoot();
	if( context != savedContext ) {
		context = savedContext;
		method = context->data[ ObjectLayout::methodInContext ];
		stack = context->data[ ObjectLayout::stackInContext ];
		byteCodes = method->data[ ObjectLayout::byteCodesInMethod ]->bytePtr();
		arguments = temporaries = literals = instanceVariables = 0;
	}
	stack->data[ stackTop++ ] = objects.nilObject;

	/*
	 * Done with primitive, continue execution loop
	 */
	return continueExecution;
}

int Interpreter::doReturn()
{
	context = context->data[ ObjectLayout::previousContextInContext ];
	return returnToContext();
}

int Interpreter::returnToContext()
{
	if( ( context == 0 ) || ( context == objects.nilObject ) ) {
		Object* aProcess = memory.popRoot();
		aProcess->data[ ObjectLayout::contextInProcess ] = context;
		aProcess->data[ ObjectLayout::resultInProcess ] = returnedValue;
		return ( OpCodes::returnReturned );
	}

	arguments = instanceVariables = literals = temporaries = 0;

	stack = context->data[ ObjectLayout::stackInContext ];
	stackTop = SmallInt::value( context->data[ ObjectLayout::stackTopInContext ] );
	stack->data[ stackTop++ ] = returnedValue;
	method = context->data[ ObjectLayout::methodInContext ];
	byteCodes = method->data[ ObjectLayout::byteCodesInMethod ]->bytePtr();
	bytePointer = SmallInt::value( context->data[ ObjectLayout::bytePointerInContext ] );
	return continueExecution;
}

int Interpreter::doSpecial( int low )
{
	switch( low ) {
	case OpCodes::selfReturn:
		if( !arguments )
			arguments = context->data[ ObjectLayout::argumentsInContext ];
		returnedValue = arguments->data[ ObjectLayout::receiverInArguments ];
		return doReturn();

	case OpCodes::stackReturn:
		returnedValue = stack->data[ --stackTop ];
		return doReturn();

	case OpCodes::blockReturn:
		returnedValue = stack->data[ --stackTop ];
		context = context->data[ ObjectLayout::creatingContextInBlock ]
					  ->data[ ObjectLayout::previousContextInContext ];
		return returnToContext();

	case OpCodes::duplicate:
		returnedValue = stack->data[ stackTop - 1 ];
		stack->data[ stackTop++ ] = returnedValue;
		break;

	case OpCodes::popTop:
		stackTop--;
		break;

	case OpCodes::branch:
		low = codeValue();
		bytePointer = low;
		break;

	case OpCodes::branchIfTrue:
		low = codeValue();
		returnedValue = stack->data[ --stackTop ];
		if( returnedValue == objects.trueObject )
			bytePointer = low;
		else
			bytePointer += codeValueSize;
		break;

	case OpCodes::branchIfFalse:
		low = codeValue();
		returnedValue = stack->data[ --stackTop ];
		if( returnedValue == objects.falseObject )
			bytePointer = low;
		else
			bytePointer += codeValueSize;
		break;

	case OpCodes::sendToSuper:
		/* next byte has literal selector number */
		low = byteCodes[ bytePointer++ ];
		if( !literals )
			literals = method->data[ ObjectLayout::literalsInMethod ];
		messageSelector = literals->data[ low ];
		receiverClass =
			method->data[ ObjectLayout::classInMethod ]
				->data[ ObjectLayout::parentClassInClass ];
		arguments = stack->data[ --stackTop ];
		sendToClass();
		break;

	case OpCodes::breakpoint: {
		/* Back up on top of the breaking location */
		bytePointer -= 1;

		/* Return to our master process */
		Object* aProcess = memory.popRoot();
		aProcess->data[ ObjectLayout::contextInProcess ] = context;
		aProcess->data[ ObjectLayout::resultInProcess ] = returnedValue;
		context->data[ ObjectLayout::bytePointerInContext ] =
			SmallInt::make( bytePointer );
		context->data[ ObjectLayout::stackTopInContext ] = SmallInt::make( stackTop );
		return ( OpCodes::returnBreak );
	}

	default:
		Log::error( "invalid doSpecial %d!", low );
		break;
	}
	return continueExecution;
}

/*
	method lookup routine, used when cache miss occurs
*/
Object* Interpreter::lookupMethod( Object* selector, Object* _class )
{
	if( !selector || selector == objects.nilObject )
		return NULL;

	if( !_class || _class == objects.nilObject )
		return NULL;

	/*
	 * Scan upward through the class hierarchy
	 */
	for( ; _class != objects.nilObject; _class = _class->data[ ObjectLayout::parentClassInClass ] ) {
		/*
		 * Consider the Dictionary of methods for this Class
		 */
		Object* dictionary = _class->data[ ObjectLayout::methodsInClass ];
		Object* keys = dictionary->data[ 0 ];
		int low = 0;
		int high = keys->size();

		/*
		 * Do a binary search through its keys, which are
		 * Symbol's.
		 */
		while( low < high ) {
			int middle = ( low + high ) / 2;
			Object* value = keys->data[ middle ];

			/*
			 * If we find the selector, return the
			 * method object.
			 */
			if( value == selector ) {
				Object* values = dictionary->data[ 1 ];
				return ( values->data[ middle ] );
			}

			/*
			 * Otherwise continue the binary search
			 */
			if( Symbol::compare( selector, value ) < 0 )
				high = middle;
			else
				low = middle + 1;
		}
	}

	/*
	 * Sorry, couldn't find a method
	 */
	return NULL;
}

/*
 * bulkReplace()
 *  Implement replaceFrom:to:with:startingAt: as a primitive
 *
 * Return 1 if we couldn't do it, 0 on success.  This routine has
 * distinct code paths for plain old byte type arrays, and for
 * arrays of object pointers; the latter must handle the special
 * case of static pointers.  It looks hairy (and it is), but it's
 * still much faster than executing the block move in Smalltalk
 * VM opcodes.
 */
int Interpreter::bulkReplace( Object* destination, Object* start, Object* stop, Object* source, Object* replacementStart )
{
	/*
	 * We only handle simple 31-bit integer indices.  Map the
	 * values onto 0-based C array type values.
	 */
	if( !SmallInt::is( replacementStart ) || !SmallInt::is( start ) ||
		!SmallInt::is( stop ) )
		return ( 1 );
	int replacementIndex = SmallInt::value( replacementStart ) - 1;
	int startIndex = SmallInt::value( start ) - 1;
	int stopIndex = SmallInt::value( stop ) - 1;
	int count = ( stopIndex - startIndex ) + 1;

	/*
	 * Defend against goofy negative indices.
	 */
	if( ( replacementIndex < 0 ) || ( startIndex < 0 ) || ( stopIndex < 0 ) || ( count < 1 ) )
		return ( 1 );

	/*
	 * Range check
	 */
	if( ( destination->size() < (uint32_t) stopIndex ) || ( source->size() < (uint32_t) ( replacementIndex + count ) ) )
		return ( 1 );

	/*
	 * If both source and destination are binary, do a memmove()
	 */
	if( source->isBinary() && destination->isBinary() ) {
		/*
		 * Do it.
		 */
		memmove( destination->bytePtr() + startIndex, source->bytePtr() + replacementIndex, (size_t) count );
		return ( 0 );
	}

	/*
	 * If not both regular storage, fail
	 */
	if( source->isBinary() || destination->isBinary() )
		return ( 1 );

	/*
	 * If we're fiddling pointers between static and dynamic memory,
	 * let the VM-based implementation deal with it.
	 */
	if( memory.isDynamic( source ) != memory.isDynamic( destination ) )
		return ( 1 );

	/*
	 * Copy object pointer fields
	 */
	memmove( &destination->data[ startIndex ], &source->data[ replacementIndex ], (size_t) ( BytesPerWord * count ) );
	return ( 0 );
}

void Interpreter::flushCache()
{
	cache.flush();
}

const MethodCache& Interpreter::methodCache() const
{
	return cache;
}
