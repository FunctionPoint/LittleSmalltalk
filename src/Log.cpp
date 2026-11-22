#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <iostream>

#include "Log.h"

Format::Format( const char* text, std::source_location where )
	: text( text ), where( where )
{
}

/*
	The bare name of the function, as __func__ gives it: function_name()
	is the whole signature on some compilers ("void __cdecl Parser::open(const char *)").
*/
void Log::bareFunctionName( const char* signature, char* name, size_t nameSize )
{
	const char* end = strchr( signature, '(' );

	if( !end )
		end = signature + strlen( signature );

	/* the name is after the last space and the last "::" before the parameters */
	const char* start = end;
	while( start > signature && start[ -1 ] != ' ' && start[ -1 ] != ':' )
		start--;

	snprintf( name, nameSize, "%.*s", (int) ( end - start ), start );
}

void Log::printLog( const Format* format, va_list arguments )
{
	char name[ 256 ];
	char output[ 2048 ];

	bareFunctionName( format->where.function_name(), name, sizeof( name ) );

	/* create the output string template */
	snprintf( output, sizeof( output ), "%s:%d %s\n", name, (int) format->where.line(), format->text );

	/* make sure it is zero terminated */
	output[ sizeof( output ) - 1 ] = 0;

	/* fill in the arguments, then print it out */
	char message[ 4096 ];
	vsnprintf( message, sizeof( message ), output, arguments );
	std::cerr << message << std::flush;
}

void Log::errorAt( const Format* format, ... )
{
	va_list arguments;

	va_start( arguments, format );
	printLog( format, arguments );
	va_end( arguments );

	exit( 1 );
}
