#pragma once

/*
	App
	The lst program: parses the command line, then compiles an image (-o)
	or loads one and runs it, with a VirtualMachine.
*/

#include <string>
#include <vector>

class VirtualMachine;

class App
{
private:
	static constexpr const char* defaultImageFile = "lst.img";
	static constexpr const char* defaultSourceFolder = "Smalltalk/System";

	std::vector<std::string> arguments;
	std::vector<const char*> argumentPointers;
	std::vector<std::string> fileNames;
	std::string outputFile;

public:
	int start( const std::vector<std::string>& commandLine );

private:
	bool parseArguments();
	static void usage();
	int buildImage( VirtualMachine& virtualMachine );
	int runImage( VirtualMachine& virtualMachine );
};
