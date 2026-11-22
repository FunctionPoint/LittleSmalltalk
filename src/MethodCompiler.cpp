#include "MethodCompiler.h"

#include <string.h>
#include <utility>

#include "OpCodes.h"

static const char* binaryBuiltIns[] = { "<", "<=", "+", 0 };

MethodCompiler::MethodCompiler( Parser& parser, std::vector<std::string> instanceVariables, std::function<bool( const char* name )> isGlobal )
	: parser( parser ), instanceVariables( std::move( instanceVariables ) ), isGlobal( std::move( isGlobal ) )
{
}

CompiledMethod MethodCompiler::compile()
{
	compileMethod();
	return std::move( currentMethod );
}

int MethodCompiler::variableTop() const
{
	return (int) variableNames.size();
}

int MethodCompiler::instanceIndex( const char* name ) const
{
	/* from the end: a variable of the class itself before those of its superclasses */
	for( int i = (int) instanceVariables.size() - 1; i >= 0; i-- )
		if( instanceVariables[ i ] == name )
			return i;
	return -1;
}

void MethodCompiler::resetMethod()
{
	currentMethod = CompiledMethod();
	argumentNames.clear();
	variableNames.clear();
	superMessage = false;
	returnOperation = 0;
	blockbackup = -1;
}

/* FIXME - remove duplicates. */
int MethodCompiler::addLiteral( const CompiledLiteral& literal )
{
	currentMethod.literals.push_back( literal );

	return (int) currentMethod.literals.size() - 1;
}

int MethodCompiler::addIntegerLiteral( int64_t value )
{
	return addLiteral( { CompiledLiteral::Integer, value, {} } );
}

int MethodCompiler::addStringLiteral( const char* text )
{
	return addLiteral( { CompiledLiteral::String, 0, text } );
}

int MethodCompiler::addSymbolLiteral( const char* text )
{
	return addLiteral( { CompiledLiteral::Symbol, 0, text } );
}

int MethodCompiler::addCharLiteral( int value )
{
	return addLiteral( { CompiledLiteral::Char, value, {} } );
}

int MethodCompiler::addGlobalLiteral( const char* name )
{
	return addLiteral( { CompiledLiteral::Global, 0, name } );
}

void MethodCompiler::genInstruction( int opcode, int argument )
{
	currentMethod.code.genInstruction( opcode, argument );
}

void MethodCompiler::addArgument( const char* name )
{
	argumentNames.push_back( name );
}

void MethodCompiler::addVariable( const char* name )
{
	variableNames.push_back( name );

	if( variableTop() > currentMethod.variableCount )
		currentMethod.variableCount = variableTop();
}

bool MethodCompiler::compileInteger( void )
{
	int64_t i = parser.readInteger();
	if( ( i >= 0 ) && ( i < 10 ) )
		genInstruction( OpCodes::pushConstant, (int) i );
	else
		genInstruction( OpCodes::pushLiteral, addIntegerLiteral( i ) );
	return true;
}

bool MethodCompiler::compilePrimitive( void )
{
	/* skip over the left bracket */
	parser.advance();
	parser.skipSpaces();

	int primitiveNumber;
	/* then read the primitive number */
	if( Parser::isDigit( parser.current() ) ) {
		int64_t number = parser.readInteger();
		if( number > 255 )
			parser.parseError( "primitive number too large" );
		primitiveNumber = (int) number;
	} else
		parser.parseError( "missing primitive number" );

	int argumentCount;
	/* then read the arguments */
	for( argumentCount = 0; parser.current() && ( parser.current() != '>' ); argumentCount++ )
		if( !compileTerm() )
			return false;

	/* make sure we ended correctly */
	if( parser.current() == '>' ) {
		parser.advance();
		parser.skipSpaces();
	} else
		parser.parseError( "missing > at end of primitive" );

	/* generate instructions */
	genInstruction( OpCodes::doPrimitive, argumentCount );
	currentMethod.code.genByte( primitiveNumber );

	/* Success */
	return true;
}

bool MethodCompiler::compileString( void )
{
	if( !parser.readString() )
		return false;
	genInstruction( OpCodes::pushLiteral, addStringLiteral( parser.token() ) );
	return true;
}

bool MethodCompiler::nameTerm( const char* name )
{
	static const char* lowConstants[ 4 ] = { "nil", "true", "false", 0 };
	/* see if method or block variable: the innermost first (a block argument hides a variable of the same name) */
	for( int i = variableTop() - 1; i >= 0; i-- ) {
		if( variableNames[ i ] == name ) {
			genInstruction( OpCodes::pushTemporary, i );
			return true;
		}
	}

	/* see if argument */
	for( int i = 0; i < (int) argumentNames.size(); i++ ) {
		if( argumentNames[ i ] == name ) {
			genInstruction( OpCodes::pushArgument, i );
			return true;
		}
	}

	/* see if super */
	if( strcmp( name, "super" ) == 0 ) {
		genInstruction( OpCodes::pushArgument, 0 );
		superMessage = true;
		return true;
	}

	/* see if low constant */
	for( int i = 0; lowConstants[ i ]; i++ ) {
		if( strcmp( lowConstants[ i ], name ) == 0 ) {
			genInstruction( OpCodes::pushConstant, 10 + i );
			return true;
		}
	}

	/* see if instance variable */
	int i = instanceIndex( name );
	if( i >= 0 ) {
		genInstruction( OpCodes::pushInstance, i );
		return true;
	}

	/* see if global */
	if( isGlobal( name ) ) {
		genInstruction( OpCodes::pushLiteral, addGlobalLiteral( name ) );
		return true;
	}

	parser.parseError( ( std::string( "unknown identifier " ) + name ).c_str() );
}

bool MethodCompiler::compileBlock( void )
{
	int savedStart = parser.position();
	parser.advance();
	parser.skipSpaces();
	genInstruction( OpCodes::pushBlock, variableTop() );
	int savedLocation = currentMethod.code.byteTop();
	currentMethod.code.genVal( 0 );

	int savedTop = variableTop();
	if( parser.current() == ':' ) {
		while( 1 ) {
			parser.advance();
			parser.skipSpaces();
			if( !Parser::isIdentifierChar( parser.current() ) )
				parser.parseError( "missing identifier" );
			if( parser.readIdentifier() )
				parser.parseError( "keyword illegal" );
			addVariable( parser.token() );
			if( parser.current() == '|' )
				break;
			if( parser.current() != ':' )
				parser.parseError( "missing colon:" );
		}
		parser.advance();
		parser.skipSpaces();
	}
	if( parser.current() == ']' )
		genInstruction( OpCodes::pushConstant, OpCodes::nilConstant );
	else {
		int savedReturnOperation = returnOperation;

		returnOperation = OpCodes::blockReturn;
		while( 1 ) {
			if( !compileStatement() )
				parser.parseError( "Statement syntax inside block" );
			if( parser.current() == '.' ) {
				parser.advance();
				parser.skipSpaces();
			}
			if( parser.current() == ']' )
				break;
			else
				genInstruction( OpCodes::doSpecial, OpCodes::popTop );
		}
		returnOperation = savedReturnOperation;
	}
	parser.advance();
	parser.skipSpaces(); /* skip over ] */
	genInstruction( OpCodes::doSpecial, OpCodes::stackReturn );
	currentMethod.code.genValPos( savedLocation, currentMethod.code.byteTop() );
	variableNames.resize( savedTop );

	/* set blockbackup to textual start of block */
	blockbackup = savedStart;
	return true;
}

bool MethodCompiler::compileSymbol( void )
{
	/* #( ... ): a literal array */
	if( parser.peek( 1 ) == '(' ) {
		parser.advance();
		genInstruction( OpCodes::pushLiteral, addLiteral( readLiteralArray() ) );
		return true;
	}

	parser.readSymbol();
	genInstruction( OpCodes::pushLiteral, addSymbolLiteral( parser.token() ) );
	return true;
}

CompiledLiteral MethodCompiler::readLiteralArray()
{
	CompiledLiteral array;

	array.kind = CompiledLiteral::Array;
	parser.advance();
	parser.skipSpaces();
	while( parser.current() != ')' )
		array.elements.push_back( readArrayElement() );
	parser.advance();
	parser.skipSpaces();
	return array;
}

CompiledLiteral MethodCompiler::readArrayElement()
{
	char character = parser.current();

	if( character == 0 )
		parser.parseError( "missing ) at end of literal array" );
	/* a nested array: ( ... ) or #( ... ) */
	if( character == '(' )
		return readLiteralArray();
	if( character == '#' && parser.peek( 1 ) == '(' ) {
		parser.advance();
		return readLiteralArray();
	}
	/* a symbol: #name, or just name (also keywords like at:put:) */
	if( character == '#' ) {
		parser.readSymbol();
		return { CompiledLiteral::Symbol, 0, parser.token() };
	}
	if( Parser::isIdentifierChar( character ) ) {
		parser.readSymbolName();
		return { CompiledLiteral::Symbol, 0, parser.token() };
	}
	if( Parser::isDigit( character ) || ( character == '-' && Parser::isDigit( parser.peek( 1 ) ) ) )
		return { CompiledLiteral::Integer, parser.readInteger(), {} };
	if( character == '$' )
		return { CompiledLiteral::Char, readCharLiteral(), {} };
	if( character == '\'' ) {
		parser.readString();
		return { CompiledLiteral::String, 0, parser.token() };
	}
	parser.parseError( "invalid literal in literal array" );
}

int MethodCompiler::readCharLiteral()
{
	/* the cursor is on the $ */
	parser.advance();
	if( parser.current() == 0 )
		parser.parseError( "missing character after $" );
	int value = (unsigned char) parser.current();
	parser.advance();
	parser.skipSpaces();
	return value;
}

bool MethodCompiler::compileChar( void )
{
	genInstruction( OpCodes::pushLiteral, addCharLiteral( readCharLiteral() ) );
	return true;
}

bool MethodCompiler::compileTerm( void )
{
	/* make it so anything other than a block zeros out backup var */
	blockbackup = -1;
	superMessage = false;

	if( parser.current() == '(' ) {
		parser.advance();
		parser.skipSpaces();
		if( !compileExpression() )
			return false;
		if( parser.current() != ')' )
			parser.parseError( "unbalanced parenthesis" );
		parser.advance();
		parser.skipSpaces();
		return true;
	}
	if( parser.current() == '<' )
		return compilePrimitive();
	if( parser.current() == '$' )
		return compileChar();
	if( Parser::isDigit( parser.current() ) || ( parser.current() == '-' && Parser::isDigit( parser.peek( 1 ) ) ) )
		return compileInteger();
	if( parser.current() == '\'' )
		return compileString();
	if( Parser::isIdentifierChar( parser.current() ) ) {
		parser.readIdentifier();
		return nameTerm( parser.token() );
	}
	if( parser.current() == '[' )
		return compileBlock();
	if( parser.current() == '#' )
		return compileSymbol();
	if( parser.current() == 0 )
		parser.parseError( "unexpected end of input" );

	parser.parseError( "illegal start of expression" );
}

bool MethodCompiler::compileUnaryContinuation( void )
{
	static const char* unaryBuiltIns[] = { "isNil", "notNil", 0 };

	while( Parser::isIdentifierChar( parser.current() ) ) {
		int savedPosition = parser.position();
		if( parser.readIdentifier() ) {
			parser.setPosition( savedPosition ); /* oops, was a keyword */
			break;
		}
		bool done = false;
		if( !superMessage ) {
			for( int i = 0; unaryBuiltIns[ i ]; i++ )
				if( strcmp( parser.token(), unaryBuiltIns[ i ] ) == 0 ) {
					genInstruction( OpCodes::sendUnary, i );
					done = true;
				}
		}
		if( !done ) {
			genInstruction( OpCodes::markArguments, 1 );
			int literalNumber = addSymbolLiteral( parser.token() );
			/*printf("unary %s\n", parser.token()); */
			if( superMessage ) {
				genInstruction( OpCodes::doSpecial, OpCodes::sendToSuper );
				currentMethod.code.genByte( literalNumber );
			} else
				genInstruction( OpCodes::sendMessage, literalNumber );
			superMessage = false;
		}
	}
	return true;
}

bool MethodCompiler::compileBinaryContinuation( void )
{
	if( !compileUnaryContinuation() )
		return false;
	while( Parser::isBinary( parser.current() ) ) {
		parser.readBinary();
		std::string selector = parser.token();
		if( !compileTerm() )
			return false;
		if( !compileUnaryContinuation() )
			return false;

		bool done = false;
		if( !superMessage )
			for( int i = 0; binaryBuiltIns[ i ]; i++ )
				if( selector == binaryBuiltIns[ i ] ) {
					genInstruction( OpCodes::sendBinary, i );
					done = true;
				}

		if( !done ) {
			int messageLiteral = addSymbolLiteral( selector.c_str() );
			genInstruction( OpCodes::markArguments, 2 );
			if( superMessage ) {
				genInstruction( OpCodes::doSpecial, OpCodes::sendToSuper );
				currentMethod.code.genByte( messageLiteral );
				superMessage = false;
			} else
				genInstruction( OpCodes::sendMessage, messageLiteral );
		}
	}
	return true;
}

bool MethodCompiler::optimizeBlock( void )
{
	if( parser.current() != '[' ) {
		if( !compileTerm() )
			return false;
		parser.parseError( "missing block as optimized block argument" );
	} else {
		parser.advance();
		parser.skipSpaces();
		if( parser.current() == ']' ) {
			genInstruction( OpCodes::pushConstant, 0 );
			parser.advance();
			parser.skipSpaces();
			return true;
		}
		while( 1 ) {
			if( !compileStatement() )
				return false;
			if( parser.current() == '.' )
				parser.advance(), parser.skipSpaces();
			if( parser.current() == ']' )
				break;
			genInstruction( OpCodes::doSpecial, OpCodes::popTop );
		}
		parser.advance();
		parser.skipSpaces();
		/* just leave last expression on stack */
	}
	return true;
}

bool MethodCompiler::controlFlow( int branchOperation, const char* rest, int defaultConstant )
{
	genInstruction( OpCodes::doSpecial, branchOperation );
	int firstJump = currentMethod.code.byteTop();
	currentMethod.code.genVal( 0 );
	if( !optimizeBlock() )
		parser.parseError( "syntax error in control flow" );
	genInstruction( OpCodes::doSpecial, OpCodes::branch );
	int secondJump = currentMethod.code.byteTop();
	currentMethod.code.genVal( 0 );
	currentMethod.code.genValPos( firstJump, currentMethod.code.byteTop() );
	int savedPosition = parser.position();
	if( Parser::isIdentifierChar( parser.current() ) && parser.readIdentifier() && ( strcmp( parser.token(), rest ) == 0 ) ) {
		if( !optimizeBlock() )
			parser.parseError( "syntax error in control cascade" );
	} else {
		parser.setPosition( savedPosition );
		genInstruction( OpCodes::pushConstant, defaultConstant );
	}
	currentMethod.code.genValPos( secondJump, currentMethod.code.byteTop() );
	return true;
}

bool MethodCompiler::optimizeLoop( int branchInstruction )
{
	/* back up to start of block and try again */
	parser.setPosition( blockbackup );
	int loopStart = currentMethod.code.byteTop();
	optimizeBlock();
	genInstruction( OpCodes::doSpecial, branchInstruction );
	int loopEnd = currentMethod.code.byteTop();
	currentMethod.code.genVal( 0 );
	if( !( Parser::isIdentifierChar( parser.current() ) && parser.readIdentifier() ) )
		parser.parseError( "can't get message again in optimized block" );
	/* now read the body */
	optimizeBlock();
	genInstruction( OpCodes::doSpecial, OpCodes::popTop );
	genInstruction( OpCodes::doSpecial, OpCodes::branch );
	currentMethod.code.genVal( loopStart );
	currentMethod.code.genValPos( loopEnd, currentMethod.code.byteTop() );
	genInstruction( OpCodes::pushConstant, 0 );
	return true;
}

bool MethodCompiler::compileKeywordContinuation( void )
{
	bool savedSuper = superMessage;
	if( !compileBinaryContinuation() )
		return false;
	int argumentCount = 0;
	std::string selector;
	if( Parser::isIdentifierChar( parser.current() ) && parser.readIdentifier() ) {
		if( strcmp( parser.token(), "ifTrue:" ) == 0 )
			return controlFlow( OpCodes::branchIfFalse, "ifFalse:", OpCodes::nilConstant );
		else if( strcmp( parser.token(), "ifFalse:" ) == 0 )
			return controlFlow( OpCodes::branchIfTrue, "ifTrue:", OpCodes::nilConstant );
		else if( strcmp( parser.token(), "and:" ) == 0 )
			return controlFlow( OpCodes::branchIfFalse, "", OpCodes::falseConstant );
		else if( strcmp( parser.token(), "or:" ) == 0 )
			return controlFlow( OpCodes::branchIfTrue, "", OpCodes::trueConstant );
		else if( ( strcmp( parser.token(), "whileTrue:" ) == 0 ) && blockbackup >= 0 )
			return optimizeLoop( OpCodes::branchIfFalse );
		else if( ( strcmp( parser.token(), "whileFalse:" ) == 0 ) && blockbackup >= 0 )
			return optimizeLoop( OpCodes::branchIfTrue );
		else
			do {
				selector += parser.token();
				argumentCount++;
				if( !compileTerm() )
					return false;
				if( !compileBinaryContinuation() )
					return false;
			} while( Parser::isIdentifierChar( parser.current() ) && parser.readIdentifier() );
	}
	if( argumentCount > 0 ) {
		bool done = false;
		if( !savedSuper )
			for( int i = 0; binaryBuiltIns[ i ]; i++ )
				if( selector == binaryBuiltIns[ i ] ) {
					genInstruction( OpCodes::sendBinary, i );
					done = true;
				}
		if( !done ) {
			genInstruction( OpCodes::markArguments, argumentCount + 1 );
			if( savedSuper ) {
				genInstruction( OpCodes::doSpecial, OpCodes::sendToSuper );
				currentMethod.code.genByte( addSymbolLiteral( selector.c_str() ) );
				superMessage = false;
			} else
				genInstruction( OpCodes::sendMessage, addSymbolLiteral( selector.c_str() ) );
		}
	}
	return true;
}

bool MethodCompiler::compileAssignment( const char* name )
{
	/* the innermost variable first, as in nameTerm */
	for( int i = variableTop() - 1; i >= 0; i-- ) {
		if( variableNames[ i ] == name ) {
			genInstruction( OpCodes::assignTemporary, i );
			return true;
		}
	}

	int i = instanceIndex( name );
	if( i >= 0 ) {
		genInstruction( OpCodes::assignInstance, i );
		return true;
	}

	parser.parseError( ( std::string( "cannot assign to " ) + name ).c_str() );
}

bool MethodCompiler::compileExpression( void )
{
	if( Parser::isIdentifierChar( parser.current() ) ) {
		parser.readIdentifier();
		if( ( ( parser.current() == '<' ) && ( parser.peek( 1 ) == '-' ) ) || ( ( parser.current() == ':' ) && ( parser.peek( 1 ) == '=' ) ) ) {
			parser.advance();
			parser.advance();
			parser.skipSpaces();
			std::string name = parser.token();
			if( !compileExpression() )
				return false;
			return compileAssignment( name.c_str() );
		}
		if( !nameTerm( parser.token() ) )
			return false;
	} else {
		if( !compileTerm() )
			return false;
	}
	if( !compileKeywordContinuation() )
		return false;
	while( parser.current() == ';' ) {
		parser.advance();
		parser.skipSpaces();
		genInstruction( OpCodes::doSpecial, OpCodes::duplicate );
		if( !compileKeywordContinuation() )
			return false;
	}
	return true;
}

bool MethodCompiler::compileStatement( void )
{
	if( parser.current() == '^' ) { /* return statement */
		parser.advance();
		parser.skipSpaces();
		if( !compileExpression() )
			return false;
		genInstruction( OpCodes::doSpecial, returnOperation );
		return true;
	}
	/* otherwise just an expression */
	if( !compileExpression() )
		return false;
	return true;
}

bool MethodCompiler::compileBody( void )
{
	returnOperation = OpCodes::stackReturn;
	while( parser.current() ) {
		if( !compileStatement() )
			return false;
		genInstruction( OpCodes::doSpecial, OpCodes::popTop );
		if( parser.current() == '.' ) {
			parser.advance();
			parser.skipSpaces();
		}
	}
	genInstruction( OpCodes::doSpecial, OpCodes::selfReturn );
	return true;
}

bool MethodCompiler::compileMethodHeader()
{
	std::string selector;

	if( Parser::isIdentifierChar( parser.current() ) ) {
		if( parser.readIdentifier() ) { /* keyword message */
			bool keywordFound = true;
			while( keywordFound ) {
				selector += parser.token();
				if( Parser::isIdentifierChar( parser.current() ) && !parser.readIdentifier() )
					addArgument( parser.token() );
				else
					parser.parseError( "missing argument after keyword" );
				int savedPosition = parser.position();
				if( Parser::isIdentifierChar( parser.current() ) && parser.readIdentifier() )
					keywordFound = true;
				else {
					parser.setPosition( savedPosition );
					keywordFound = false;
				}
			}
		} else
			selector = parser.token();					/* unary message */
	} else if( Parser::isBinary( parser.current() ) ) { /* binary message */
		parser.readBinary();
		selector = parser.token();
		if( !Parser::isIdentifierChar( parser.current() ) )
			parser.parseError( "missing argument" );
		parser.readIdentifier();
		addArgument( parser.token() );
	} else
		parser.parseError( "ill formed method header" );
	currentMethod.selector = selector;
	return true;
}

bool MethodCompiler::compileMethodVariables( void )
{
	variableNames.clear();
	currentMethod.variableCount = 0;
	if( parser.current() != '|' )
		return true;
	parser.advance();
	parser.skipSpaces();
	while( parser.current() != '|' ) {
		if( !Parser::isIdentifierChar( parser.current() ) )
			parser.parseError( "need identifier" );
		if( parser.readIdentifier() )
			parser.parseError( "keyword illegal" );
		addVariable( parser.token() );
	}
	parser.advance();
	parser.skipSpaces();
	return true;
}

bool MethodCompiler::compileMethod()
{
	resetMethod();

	/* argument 0 is always the receiver */
	addArgument( "self" );

	if( !compileMethodHeader() )
		return false;
	if( !compileMethodVariables() )
		return false;
	if( compileBody() ) {
		currentMethod.stackSize = 19;
		return true;
	}
	return false;
}
