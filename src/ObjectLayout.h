#pragma once

/*
	ObjectLayout
	The field offsets of the objects the VM (and the bootstrapper) know
	the layout of: Process, Context, Block, Method, Class, Node,
	Dictionary...
*/

class ObjectLayout
{
public:
	/*
		The following are the objects with ``hard-wired''
		field offsets
	*/
	/*
		A Process has three fields
			* a current context
			* status of process (running, waiting, etc)
			* the result
	*/
	static constexpr int processSize = 3;
	static constexpr int contextInProcess = 0;
	static constexpr int statusInProcess = 1;
	static constexpr int resultInProcess = 2;

	/*
		A Context has:
			* method (which has bytecode pointer)
			* arguments - an array
			* temporaries - an array
			* stack - an array
			* bytecode offset (an integer)
			* stack pointer
			* a reference to the previous context in the chain.
	*/

	static constexpr int contextSize = 7;
	static constexpr int methodInContext = 0;
	static constexpr int argumentsInContext = 1;
	static constexpr int temporariesInContext = 2;
	static constexpr int stackInContext = 3;
	static constexpr int bytePointerInContext = 4;
	static constexpr int stackTopInContext = 5;
	static constexpr int previousContextInContext = 6;

	/*
		A Block is subclassed from Context
		shares fields with creator, plus a couple new ones
	*/

	static constexpr int blockSize = 10;
	static constexpr int methodInBlock = methodInContext;
	static constexpr int argumentsInBlock = argumentsInContext;
	static constexpr int temporariesInBlock = temporariesInContext;
	static constexpr int stackInBlock = stackInContext;
	static constexpr int stackTopInBlock = stackTopInContext;
	static constexpr int previousContextInBlock = previousContextInContext;
	static constexpr int argumentLocationInBlock = 7;
	static constexpr int creatingContextInBlock = 8;
	static constexpr int bytePointerInBlock = 9;

	/*
		A Method has:
			* name (a Symbol)
			* bytecodes
			* literals
			* stack size
			* temp size
			* class that owns the method
	*/

	static constexpr int methodSize = 6;
	static constexpr int nameInMethod = 0;
	static constexpr int byteCodesInMethod = 1;
	static constexpr int literalsInMethod = 2;
	static constexpr int stackSizeInMethod = 3;
	static constexpr int temporarySizeInMethod = 4;
	static constexpr int classInMethod = 5;

	/*
		A Class has:
			* pointer to parent class
			* pointer to tree of methods
	*/

	static constexpr int classSize = 5;
	static constexpr int nameInClass = 0;
	static constexpr int parentClassInClass = 1;
	static constexpr int methodsInClass = 2;
	static constexpr int instanceSizeInClass = 3;
	static constexpr int variablesInClass = 4;

	/*
		A node in a tree has
			* value field
			* left subtree
			* right subtree
	*/

	static constexpr int valueInNode = 0;
	static constexpr int leftInNode = 1;
	static constexpr int rightInNode = 2;

	/*
		misc defines
	*/

	static constexpr int rootInTree = 0;
	static constexpr int receiverInArguments = 0;
	static constexpr int symbolsInSymbol = 5;

	/* Dictionary */
	static constexpr int keysInDictionary = 0;
	static constexpr int valuesInDictionary = 1;
};
