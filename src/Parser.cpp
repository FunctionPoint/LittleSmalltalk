#include "Parser.h"

#include <stddef.h>
#include <string.h>

#include "Log.h"

Parser::~Parser()
{
	if( inputFile )
		fclose( inputFile );
}

void Parser::open( const char* fileName )
{
	if( inputFile )
		fclose( inputFile );
	if( ( inputFile = fopen( fileName, "r" ) ) == NULL )
		Log::error( "Unable to open file \"%s\"!", fileName );
	inputLine = 0;
}

/* read the next line, skip leading spaces and comments */
bool Parser::nextLine()
{
	if( !fgets( inputBuffer, sizeof( inputBuffer ), inputFile ) )
		return false;

	cursor = inputBuffer;
	skipSpaces();
	inputLine++;
	inputPosition = 0;
	return true;
}

void Parser::skipSpaces()
{
	while( ( *cursor == ' ' ) || ( *cursor == '\t' ) || ( *cursor == '\n' ) || ( *cursor == '\r' ) )
		cursor++;
	if( *cursor == '\"' ) {
		cursor++;
		while( *cursor && ( *cursor != '\"' ) )
			cursor++;
		if( *cursor != '\"' )
			parseError( "unterminated comment" );
		cursor++;
		skipSpaces();
	}
}

char Parser::current() const
{
	return *cursor;
}

/* Remove whitespace at the end of the line too. */
void Parser::trimLine()
{
	int i = (int) strlen( inputBuffer ) - 1;

	while( i > 0 )
		if( ( inputBuffer[ i ] == ' ' ) || ( inputBuffer[ i ] == '\t' ) || ( inputBuffer[ i ] == '\n' ) )
			inputBuffer[ i-- ] = 0;
		else
			break;
}

void Parser::advance()
{
	cursor++;
}

const char* Parser::text() const
{
	return inputBuffer;
}

/* replace the current text, and put the cursor at its start */
void Parser::setText( const char* text )
{
	cursor = inputBuffer;
	if( strlen( text ) >= sizeof( inputBuffer ) ) {
		inputBuffer[ 0 ] = '\0';
		parseError( "method text too long" );
	}
	strcpy( inputBuffer, text );
}

bool Parser::isIdentifierChar( char character )
{
	if( ( character >= 'a' ) && ( character <= 'z' ) )
		return true;
	if( ( character >= 'A' ) && ( character <= 'Z' ) )
		return true;
	return false;
}

bool Parser::readIdentifier()
{
	char* tokenEnd = tokenBuffer;

	while( ( tokenEnd < ( tokenBuffer + tokenBufferSize ) ) && ( isIdentifierChar( *cursor ) || isDigit( *cursor ) ) )
		*tokenEnd++ = *cursor++;

	if( tokenEnd >= ( tokenBuffer + tokenBufferSize - 1 ) )
		parseError( "identifier too long" );

	bool keywordFound;
	*tokenEnd = '\0';
	if( *cursor == ':' ) { /* It's a keyword identifier */
		keywordFound = true;
		*tokenEnd++ = ':';
		*tokenEnd = '\0';
		cursor++;
	} else
		keywordFound = false;
	skipSpaces();
	return keywordFound;
}

bool Parser::isDigit( char character )
{
	if( ( character >= '0' ) && ( character <= '9' ) )
		return true;
	return false;
}

const char* Parser::token() const
{
	return tokenBuffer;
}

void Parser::readMethodText()
{
	bool foundExclamation = false;

	cursor = inputBuffer;
	inputPosition = 0;

	while( (ptrdiff_t) ( cursor - inputBuffer ) < (ptrdiff_t) ( sizeof( inputBuffer ) ) ) {
		int inputChar = fgetc( inputFile );

		if( inputChar == EOF )
			Log::error( "Unexpected EOF while reading method text at line %d!", inputLine );

		if( inputChar == '\n' ) {
			inputPosition = 0;
			inputLine++;
		} else
			inputPosition++;

		*cursor++ = (char) inputChar;

		/* only check if this is the first character on a line. */
		if( foundExclamation )
			if( inputPosition == 0 && inputChar == '\n' ) {
				*( cursor - 2 ) = (char) 0;
				return;
			} else
				foundExclamation = false;
		else if( inputPosition == 1 && inputChar == '!' )
			foundExclamation = true;
	}
}

int Parser::line() const
{
	return inputLine;
}

/* put the cursor back at the start of the current text */
void Parser::rewind()
{
	cursor = inputBuffer;
}

bool Parser::isBinary( char character )
{
	switch( character ) {
	case '+':
	case '*':
	case '-':
	case '/':
	case '<':
	case '=':
	case '>':
	case '@':
	case '~':
	case ',':
	case '&':
	case '|':
	case '%':
	case '\\':
	case '?':
		return true;
	}
	return false;
}

void Parser::readBinary()
{
	tokenBuffer[ 0 ] = *cursor++;
	if( isBinary( *cursor ) ) {
		tokenBuffer[ 1 ] = *cursor++;
		tokenBuffer[ 2 ] = '\0';
	} else
		tokenBuffer[ 1 ] = '\0';
	skipSpaces();
}

char Parser::peek( int offset ) const
{
	return cursor[ offset ];
}

int64_t Parser::readInteger()
{
	int64_t value = 0;
	bool negative = false;

	if( *cursor == '-' ) {
		negative = true;
		++cursor;
	}
	while( isDigit( *cursor ) ) {
		if( value > ( INT64_MAX - 9 ) / 10 )
			parseError( "integer too large" );
		value = 10 * value + ( *cursor++ - '0' );
	}
	skipSpaces();
	return negative ? -value : value;
}

/* the cursor is on the opening quote */
bool Parser::readString()
{
	cursor++;
	char* tokenEnd;
	for( tokenEnd = tokenBuffer; tokenEnd < ( tokenBuffer + tokenBufferSize - 1 ) && *cursor; ) {
		if( *cursor == '\'' ) {
			/* '' is a quote in the string */
			if( cursor[ 1 ] != '\'' )
				break;
			cursor++;
		}
		*tokenEnd++ = *cursor++;
	}

	if( tokenEnd >= ( tokenBuffer + tokenBufferSize - 1 ) )
		parseError( "string too long" );

	if( *cursor != '\'' )
		parseError( "missing end of string" );
	cursor++;
	skipSpaces();
	*tokenEnd = '\0';
	return true;
}

/* the cursor is on the # */
void Parser::readSymbol()
{
	cursor++;
	readSymbolName();
}

/* the cursor is on the first character of the name */
void Parser::readSymbolName()
{
	char* tokenEnd = tokenBuffer;
	char* tokenLimit = tokenBuffer + tokenBufferSize - 1;

	if( isIdentifierChar( *cursor ) ) {
		/* a name or keywords: foo, at:put: */
		while( tokenEnd < tokenLimit && ( isIdentifierChar( *cursor ) || isDigit( *cursor ) || ( *cursor == ':' ) ) )
			*tokenEnd++ = *cursor++;
	} else if( isBinary( *cursor ) ) {
		/* a binary selector of one or two characters */
		*tokenEnd++ = *cursor++;
		if( isBinary( *cursor ) )
			*tokenEnd++ = *cursor++;
	} else
		parseError( "invalid symbol" );
	*tokenEnd = '\0';
	skipSpaces();
}

int Parser::position() const
{
	return (int) ( cursor - inputBuffer );
}

void Parser::setPosition( int position )
{
	cursor = inputBuffer + position;
}

void Parser::parseError( const char* message )
{
	throw CompileError{ message, textLine() };
}

int Parser::textLine() const
{
	int lineNumber = 1;
	const char* end = cursor;

	/* the line of the last token: the cursor has already skipped the white space after it */
	while( end > inputBuffer && ( end[ -1 ] == ' ' || end[ -1 ] == '\t' || end[ -1 ] == '\n' || end[ -1 ] == '\r' ) )
		end--;
	for( const char* character = inputBuffer; character < end && *character; character++ )
		if( *character == '\n' )
			lineNumber++;
	return lineNumber;
}
