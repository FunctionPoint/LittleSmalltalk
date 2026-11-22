/*
	Little Smalltalk, version 4
	Written by Tim Budd, Oregon State University, budd@cs.orst.edu
	All rights reserved, no guarantees given whatsoever.
	May be freely redistributed if not for profit.
*/

#pragma once

/*
	OpCodes
	The bytecodes, shared by the Interpreter and the compiler: the opcodes,
	the special operations (opcode doSpecial), the constants of pushConstant
	and the codes Interpreter::execute returns.
*/

class OpCodes
{
public:
	/* opcodes */
	static constexpr int extended = 0;
	static constexpr int pushInstance = 1;
	static constexpr int pushArgument = 2;
	static constexpr int pushTemporary = 3;
	static constexpr int pushLiteral = 4;
	static constexpr int pushConstant = 5;
	static constexpr int assignInstance = 6;
	static constexpr int assignTemporary = 7;
	static constexpr int markArguments = 8;
	static constexpr int sendMessage = 9;
	static constexpr int sendUnary = 10;
	static constexpr int sendBinary = 11;
	static constexpr int pushBlock = 12;
	static constexpr int doPrimitive = 13;
	static constexpr int doSpecial = 15;

	/* special operations (opcode doSpecial) */
	static constexpr int selfReturn = 1;
	static constexpr int stackReturn = 2;
	static constexpr int blockReturn = 3;
	static constexpr int duplicate = 4;
	static constexpr int popTop = 5;
	static constexpr int branch = 6;
	static constexpr int branchIfTrue = 7;
	static constexpr int branchIfFalse = 8;
	static constexpr int sendToSuper = 11;
	static constexpr int breakpoint = 12;

	/* constants of pushConstant: 0 to 9 are the integers 0 to 9 */
	static constexpr int nilConstant = 10;
	static constexpr int trueConstant = 11;
	static constexpr int falseConstant = 12;

	/* the codes Interpreter::execute returns */
	static constexpr int returnError = 2;
	static constexpr int returnBadMethod = 3;
	static constexpr int returnReturned = 4;
	static constexpr int returnTimeExpired = 5;
	static constexpr int returnBreak = 6;
};
