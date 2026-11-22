#pragma once

/*
	Log
	Error messages to stderr, starting with the calling function's name and
	line: Log::error( ... ) prints the message, then it exits. The arguments
	are like printf's:
		Log::error( "Unable to open file \"%s\"!", fileName );
	The caller's function and line come from std::source_location: the
	format string is converted to a Format at the call site, and its
	constructor's default argument std::source_location::current() is
	evaluated there.
*/

#include <stdarg.h>
#include <stddef.h>
#include <source_location>

/* the format string of a log message, and where the message comes from */
class Format
{
public:
	const char* text;
	std::source_location where;

	Format( const char* text, std::source_location where = std::source_location::current() );
};

class Log
{
public:
	template <typename... Arguments>
	[[noreturn]] static void error( Format format, Arguments... arguments );

private:
	/* the printf-style printing, in Log.cpp */
	[[noreturn]] static void errorAt( const Format* format, ... );
	static void printLog( const Format* format, va_list arguments );
	static void bareFunctionName( const char* signature, char* name, size_t nameSize );
};

/*
	Templates have to be defined in the header: this one only passes its
	arguments on to the C varargs function in Log.cpp.
*/

template <typename... Arguments>
void Log::error( Format format, Arguments... arguments )
{
	errorAt( &format, arguments... );
}
