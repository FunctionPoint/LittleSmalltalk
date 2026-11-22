#include <iostream>
#include <string>
#include <vector>

#include "App.h"
#include "VirtualMachine.h"
#include "version.h"

int App::start( const std::vector<std::string>& commandLine )
{
	/* all arguments, the program name first: the image gets them with primitive 170 */
	arguments = commandLine;
	for( const std::string& argument : arguments )
		argumentPointers.push_back( argument.c_str() );

	if( !parseArguments() ) {
		usage();
		return 1;
	}

	VirtualMachine virtualMachine;

	/* with -o, fileNames are the source folders to compile into the image outputFile; that is all */
	if( !outputFile.empty() )
		return buildImage( virtualMachine );
	return runImage( virtualMachine );
}

bool App::parseArguments()
{
	/* the image, or with -o the Smalltalk source folders; the program name is skipped */
	for( size_t i = 1; i < arguments.size(); i++ ) {
		const std::string& option = arguments[ i ];
		bool hasValue = i + 1 < arguments.size();

		if( option == "-v" )
			std::cout << "Little Smalltalk, version " VERSION_STRING "\n";
		else if( option == "-o" && hasValue )
			outputFile = arguments[ ++i ];
		else if( option.starts_with( '-' ) )
			return false;
		else
			fileNames.push_back( option );
	}

	/* without -o there is only an image */
	return !outputFile.empty() || fileNames.size() <= 1;
}

void App::usage()
{
	std::cout << "Usage: lst [-v] [image]\n";
	std::cout << "       lst [-v] -o outputfile [sourcefolder...]\n";
	std::cout << "\t-v\tPrint out the Little Smalltalk version.\n";
	std::cout << "\t-o outputfile\tCompile the Smalltalk source folders, in order, into the image outputfile (lst does not run it).\n";
	std::cout << "\timage\tThe image to run (default \"" << defaultImageFile << "\").\n";
	std::cout << "\tsourcefolder\tWith -o: a folder of Smalltalk source files (*.st, in fileIn format; its subfolders too), compiled in the order of their paths (default \"" << defaultSourceFolder << "\").\n";
}

int App::buildImage( VirtualMachine& virtualMachine )
{
	if( fileNames.empty() )
		fileNames.push_back( defaultSourceFolder );
	int objectCount = virtualMachine.buildImage( fileNames, outputFile.c_str() );

	std::cout << objectCount << " objects written to \"" << outputFile << "\".\n";
	return 0;
}

int App::runImage( VirtualMachine& virtualMachine )
{
	virtualMachine.setArguments( (int) argumentPointers.size(), argumentPointers.data() );
	virtualMachine.loadImage( fileNames.empty() ? defaultImageFile : fileNames[ 0 ].c_str() );

	return virtualMachine.run();
}
