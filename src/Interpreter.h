/*
	Little Smalltalk, version 4
	Written by Tim Budd, Oregon State University, budd@cs.orst.edu
	All rights reserved, no guarantees given whatsoever.
	May be freely redistributed if not for profit.
*/

#pragma once

/*
	Interpreter
	The bytecode interpreter: given a process object, executes bytecodes
	in a tight loop. Calls out for garbage collection (ObjectMemory),
	finding a method that is not in the MethodCache, and the Primitives it
	does not do itself.
*/

#include <stdint.h>

#include "MethodCache.h"

class Object;
class Diagnostics;
class ObjectMemory;
class Primitives;
class SpecialObjects;

class Interpreter
{
private:
	static constexpr int continueExecution = 0;
	static constexpr int primitiveSucceeded = -1;
	static constexpr int primitiveFailed = -2;
	static constexpr int codeValueSize = 2;

	ObjectMemory& memory;
	SpecialObjects& objects;
	Primitives& primitives;
	const Diagnostics& diagnostics;
	MethodCache cache;

	/* the registers of the process being executed */
	Object* context = nullptr;
	Object* method = nullptr;
	Object* arguments = nullptr;
	Object* temporaries = nullptr;
	Object* instanceVariables = nullptr;
	Object* literals = nullptr;
	Object* stack = nullptr;
	int stackTop = 0;
	uint8_t* byteCodes = nullptr;
	int bytePointer = 0;
	Object* returnedValue = nullptr;
	Object* messageSelector = nullptr;
	Object* receiverClass = nullptr;

public:
	Interpreter( ObjectMemory& memory, SpecialObjects& objects, Primitives& primitives, const Diagnostics& diagnostics );
	Interpreter( const Interpreter& ) = delete;
	Interpreter& operator=( const Interpreter& ) = delete;

	int execute( Object* aProcess, int ticks );

	void flushCache();
	const MethodCache& methodCache() const;

private:
	void pushInstance( int low );
	void pushArgument( int low );
	void pushTemporary( int low );
	void pushLiteral( int low );
	void pushConstant( int low );
	void pushBlock( int low );
	int codeValue() const;
	void assignInstance( int low );
	void assignTemporary( int low );
	void markArguments( int low );
	void sendMessage( int low );
	void send();
	void sendToClass();
	void sendUnary( int low );
	void sendBinary( int low );
	int doPrimitive( int argumentCount );
	int objectIdentity();
	int classOfObject();
	int printCharacter();
	int objectSize();
	int arrayAtPut();
	int executeProcess();
	int newObject();
	int invokeBlock( int argumentCount );
	int readCharacter();
	int smallIntAdd();
	Object* smallIntResult( int64_t result );
	bool popTwoSmallInts( int& high, int& low );
	int smallIntQuotient();
	int smallIntRemainder();
	int smallIntLessThan();
	int smallIntEqual();
	int smallIntMultiply();
	int smallIntSubtract();
	int errorTrap();
	int newByteArray();
	int stringAt();
	int stringAtPut();
	int stringClone();
	int arrayAt();
	int integerOperation( int number );
	int newInteger();
	int integerLowOrder();
	int flushMethodCache();
	int exchangeObjects();
	int bitOr();
	int bitAnd();
	int replaceFrom();
	int bitShift();
	int integerTruncated();
	int callPrimitive( int number, int argumentCount );
	int failPrimitive();
	int doReturn();
	int returnToContext();
	int doSpecial( int low );

	Object* lookupMethod( Object* selector, Object* _class );
	int bulkReplace( Object* destination, Object* start, Object* stop, Object* source, Object* replacementStart );
};
