cmake_minimum_required(VERSION 3.21)

project(LittleSmalltalk VERSION 6.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(VERSION_MAJOR ${PROJECT_VERSION_MAJOR})
set(VERSION_MINOR ${PROJECT_VERSION_MINOR})
set(VERSION_PATCH ${PROJECT_VERSION_PATCH})
set(VERSION "${PROJECT_VERSION}")

# make the version file.
configure_file("${PROJECT_SOURCE_DIR}/src/version.h.in" "${PROJECT_SOURCE_DIR}/src/version.h" @ONLY)

# the VM, with the compiler that builds an image from a Smalltalk source file (lst -o).
set(VM_SOURCES
	src/App.cpp
	src/App.h
	src/Bytecode.cpp
	src/Bytecode.h
	src/CompiledClass.cpp
	src/CompiledClass.h
	src/CompiledImage.cpp
	src/CompiledImage.h
	src/CompiledLiteral.h
	src/CompiledMethod.h
	src/Compiler.cpp
	src/Compiler.h
	src/Diagnostics.cpp
	src/Diagnostics.h
	src/Image.cpp
	src/Image.h
	src/Interpreter.cpp
	src/Interpreter.h
	src/Log.cpp
	src/Log.h
	src/main.cpp
	src/MethodCache.cpp
	src/MethodCache.h
	src/MethodCompiler.cpp
	src/MethodCompiler.h
	src/Object.cpp
	src/Object.h
	src/ObjectLayout.h
	src/ObjectMemory.cpp
	src/ObjectMemory.h
	src/OpCodes.h
	src/Parser.cpp
	src/Parser.h
	src/platform.h
	src/Primitives.cpp
	src/Primitives.h
	src/RuntimeCompiler.cpp
	src/RuntimeCompiler.h
	src/SpecialObjects.cpp
	src/SpecialObjects.h
	src/version.h
	src/VirtualMachine.cpp
	src/VirtualMachine.h
)

add_executable(lst ${VM_SOURCES})

# after every build, copy the executable to bin/ (next to the image bin/lst.img), where the
# scripts of the examples run it: the configuration built last (Debug or Release) is the one there
add_custom_command(TARGET lst POST_BUILD
	COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:lst> ${PROJECT_SOURCE_DIR}/bin/
	COMMENT "Copying lst to bin/." VERBATIM
)

# Link-time optimization in Release: all member functions are in .cpp files, so without it
# the small ones on the interpreter's hot path (Object::size(), ObjectMemory::allocate()...)
# are not inlined, and the VM is about 40% slower.
include(CheckIPOSupported)
check_ipo_supported(RESULT LST_IPO_SUPPORTED OUTPUT LST_IPO_MESSAGE LANGUAGES CXX)
if(NOT LST_IPO_SUPPORTED)
	message(STATUS "Link-time optimization not supported: ${LST_IPO_MESSAGE}")
endif()

if(LST_IPO_SUPPORTED)
	set_property(TARGET lst PROPERTY INTERPROCEDURAL_OPTIMIZATION_RELEASE ON)
endif()
if(MSVC)
	target_compile_options(lst PRIVATE /W3 /permissive- /Zc:__cplusplus /utf-8)
	# The code uses the POSIX names (strdup, unlink...) and the classic CRT functions (fopen, strcpy...).
	target_compile_definitions(lst PRIVATE _CRT_SECURE_NO_WARNINGS _CRT_NONSTDC_NO_DEPRECATE)
	# Just My Code: the debugger steps over CRT code.
	target_compile_options(lst PRIVATE $<$<CONFIG:Debug>:/JMC>)
else()
	target_compile_options(lst PRIVATE -Wall)
endif()

# the Smalltalk sources of the REPL image: one file per class
file(GLOB_RECURSE SYSTEM_SOURCES CONFIGURE_DEPENDS ${PROJECT_SOURCE_DIR}/Smalltalk/System/*.st)

# bootstrap the initial image bin/lst.img (the default image name): lst -o compiles the source folder
# Smalltalk/System into the image and exits.
add_custom_target(baseimage ALL
	COMMAND ${CMAKE_COMMAND} -DLST=$<TARGET_FILE:lst> -DSOURCE=${PROJECT_SOURCE_DIR}/Smalltalk/System -DIMAGE=${PROJECT_SOURCE_DIR}/bin/lst.img -P ${PROJECT_SOURCE_DIR}/cmake/baseimage.cmake
	WORKING_DIRECTORY ${PROJECT_BINARY_DIR}
	COMMENT "Bootstrapping initial image file." VERBATIM
	SOURCES ${SYSTEM_SOURCES} ${PROJECT_SOURCE_DIR}/cmake/baseimage.cmake
)
add_dependencies(baseimage lst)
