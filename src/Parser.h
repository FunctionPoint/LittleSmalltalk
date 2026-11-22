#pragma once

/*
	Parser
	Reads the Smalltalk source file line by line (or a whole method at a
	time) and splits the current text into tokens. The text buffer and the
	cursor into it stay private: the Compiler looks at the current
	character (current, peek), moves on (advance), and saves and restores
	the cursor as a position (position, setPosition).
*/

#include <stdint.h>
#include <stdio.h>
#include <string>

/* a syntax error: what is wrong, and the line in the text (1 for the first line) */
struct CompileError {
	std::string message;
	int line;
};

class Parser
{
public:
	static constexpr int inputBufferSize = 8192;
	static constexpr int tokenBufferSize = inputBufferSize;

private:
	FILE* inputFile = nullptr;
	char inputBuffer[ inputBufferSize ] = {};
	int inputLine = 0;
	int inputPosition = 0;
	char* cursor = inputBuffer;
	char tokenBuffer[ tokenBufferSize ] = {};

public:
	Parser() = default;
	~Parser();
	Parser( const Parser& ) = delete;
	Parser& operator=( const Parser& ) = delete;

	/* input */
	void open( const char* fileName );
	bool nextLine();
	void readMethodText();
	void setText( const char* text );
	void rewind();

	/* tokens */
	void skipSpaces();
	void trimLine();
	void readBinary();
	bool readIdentifier();
	int64_t readInteger();
	[[noreturn]] void parseError( const char* message );

	bool readString();
	void readSymbol();
	void readSymbolName();

	static bool isDigit( char character );
	static bool isIdentifierChar( char character );
	static bool isBinary( char character );

	/* cursor */
	char current() const;
	char peek( int offset ) const;
	void advance();
	int position() const;
	void setPosition( int position );

	/* state (read only) */
	const char* token() const;
	const char* text() const;
	int line() const;
	int textLine() const;
};
