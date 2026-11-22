#!/bin/bash
# Build lst with CMake (Release, or Debug with: build.sh debug), using the
# preset of this platform (Windows: msvc, Linux: linux, macOS: macos); the
# build also makes the REPL image bin/lst.img (the baseimage step) and copies
# lst to bin. Then build the test image bin/test.img (the system classes and
# the unit tests) and run the unit tests.
# Always works on the folder this script is in.

set -e
cd "$(dirname "$0")"

configuration=release
if [ "$1" = debug ]; then
	configuration=debug
elif [ -n "$1" ] && [ "$1" != release ]; then
	echo "usage: $0 [debug|release]" >&2
	exit 1
fi

case "$(uname -s)" in
MINGW* | MSYS* | CYGWIN*)
	configurePreset=msvc
	buildPreset=$configuration
	;;
Linux)
	configurePreset=linux
	buildPreset=linux-$configuration
	;;
Darwin)
	configurePreset=macos
	buildPreset=macos-$configuration
	;;
*)
	echo "unknown platform $(uname -s)" >&2
	exit 1
	;;
esac

# on Windows cmake is not on the PATH: use the copy bundled with Visual Studio
cmake=cmake
if ! command -v "$cmake" > /dev/null; then
	cmake="C:/Program Files/Microsoft Visual Studio/18/Community/Common7/IDE/CommonExtensions/Microsoft/CMake/CMake/bin/cmake.exe"
	if [ ! -x "$cmake" ]; then
		echo "cmake not found (see README.md)" >&2
		exit 1
	fi
fi

"$cmake" --preset "$configurePreset"
"$cmake" --build --preset "$buildPreset"

lst=bin/lst
"$lst" -o bin/test.img Smalltalk/System Smalltalk/Test
"$lst" bin/test.img
