#pragma once

/*
	Bytecode
	The bytecodes of the method being compiled: generates instructions
	and values, patches values in place (jump targets), and builds the
	ByteArray object for the method.
*/

#include <stdint.h>
#include <vector>

class Object;
class CompiledImage;

class Bytecode
{
private:
	std::vector<uint8_t> bytes;

public:
	void clear();
	int byteTop() const;
	const std::vector<uint8_t>& byteCodes() const;

	void genByte( int byte );
	void genVal( int value );
	void genValPos( int position, int value );
	void genInstruction( int opcode, int argument );

	Object* buildByteArray( CompiledImage& image ) const;
};
