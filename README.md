# Little Smalltalk

Little Smalltalk is a small, simple dialect of Smalltalk: a bytecode virtual machine, a compiler, and a set of classes you can read in an afternoon. It does not follow the Smalltalk-80 standard, and it is not fast, but it is complete and small enough to understand the whole language implementation: object memory and garbage collection, compiling, interpreting and bootstrapping.

This is version 6.0.0, a reworked fork of [kyle-github/littlesmalltalk](https://github.com/kyle-github/littlesmalltalk) (version 4.7.1), which itself goes back to Timothy Budd's Little Smalltalk 4.0 with changes by Andy Valencia and Kyle Hayes. See that repository for the history and the older versions.

- [Changes](#changes)
- [Building](#building)
- [Running](#running)
- [Examples](#examples)
- [Unit tests](#unit-tests)
- [Folders](#folders)
- [Credits and license](#credits-and-license)

## Changes

- **Converted to C++.** The VM and the compiler are C++20, organized in classes (one class per `.h`/`.cpp` pair), and build without warnings.
- **Bootstrap and VM integrated.** There is one executable, `lst`. It runs an image, and `lst -o` compiles Smalltalk sources into a new image; there is no separate bootstrap program any more.
- **One Smalltalk compiler.** The image no longer contains a second compiler written in Smalltalk. Everything the running image compiles (expressions typed in the REPL, `File fileIn:`, `Class>>addMethod:`) goes through the same C++ compiler that bootstraps the image, called from Smalltalk through a primitive.
- **One file per class.** The Smalltalk sources are split into one `.st` file per class, named after the class, so a class is easy to find and edit.
- **Classes organized in folders.** The system classes are grouped in `Smalltalk/System/Base`, `Collection`, `Magnitude`, `OS` and `Reflection`.
- **Cleaned-up, more readable Smalltalk code.** Consistent layout (tab indentation, spaces inside parentheses and brackets, temporaries on their own line, each method ending with a period) and full variable names instead of abbreviations and single letters.
- **Bugs fixed** in the VM, the compiler and the system classes, many of them found by the new unit tests.
- **VS Code build system.** CMake with presets, IntelliSense through CMake Tools, and launch configurations that build an image and run the unit tests or the shell in the debugger.
- **Customizable startup class.** If an image has a class `Main` with a `start` method, `lst` runs `Main new start` and exits when it returns; without one, it starts the shell (the REPL, class `Shell`). A program is just a set of classes plus a `Main`.
- **Unit tests for the Smalltalk code.** A test class for every system class, and a small test framework (class `Test`) that is part of every image, so programs can have their own tests.
- **No source code in the image.** Methods are stored as bytecodes only; the `.st` files are the source, and you edit them in your IDE. The method editor and the file-out methods were removed with it, and the image is smaller (about 2100 objects).
- **Memory that grows when needed.** The object memory dynamically grows and no longer has a fixed size set on the command line (the `-s` and `-d` options are gone).

## Building

The build uses CMake (3.21 or later for the presets). Every build writes the executable to `build/Debug` or `build/Release`, copies it to `bin/`, and bootstraps the shell image `bin/lst.img` from `Smalltalk/System`.

- **Windows** (Visual Studio 2026): open the folder in VS Code and build with F7, or use the CMake that comes with Visual Studio:

  ```sh
  cmake --preset msvc
  cmake --build --preset release      # or: --preset debug
  ```

- **Linux and macOS** (GCC or Clang, with Ninja): presets `linux` and `macos`, then `linux-release` or `macos-release` (or the `-debug` variants).

## Running

Start the shell, the read-eval-print loop:

```sh
$> ./bin/lst ./bin/lst.img
Welcome to Little Smalltalk!
-> 3 + 4
7
-> 'hello' reverse
olleh
```

The shell prints the value of every expression you type.

`lst.img` is the default image name, so `lst` started in `bin/` without arguments does the same. The shell reads its commands from standard input, one per line, so commands can also come from a file: `./bin/lst ./bin/lst.img < commands.txt`.

Build an image from Smalltalk sources with `-o`. It compiles the `*.st` files of one or more source folders, including their subfolders (default: `Smalltalk/System`), writes the image and exits; then run the image:

```sh
$> ./bin/lst -o ./primes.img ./Smalltalk/System ./Smalltalk/Examples/Primes
$> ./bin/lst ./primes.img
2 3 5 7 11 13 17 19 23 29
```

A program brings its own `Main.st`, for example `Smalltalk/Examples/Primes/Main.st`:

```smalltalk
+Application subclass: #Main variables: #( ) classVariables: #( )
!Main
start
	Number primes: 9.
!
```

Options: `lst [-v] [image]` to run an image, `lst [-v] -o outputfile [sourcefolder...]` to build one; `-v` prints the version. There are no memory size options: the memory grows by itself when it is full, both for a running image and while `-o` builds one.

## Examples

`Smalltalk/Examples` has five small programs: Concordance, Palindrome, Primes, Queen (eight queens) and While. Each folder has two bash scripts that work from any folder: `build.sh` builds the example's image `lst.img` in its folder, and `start.sh` runs it (with `bin/lst`, so build `lst` first):

```sh
$> ./Smalltalk/Examples/Queen/build.sh
$> ./Smalltalk/Examples/Queen/start.sh
```

Concordance reads text from standard input; its `start.sh` uses `Input.txt` when no input is piped or redirected to it.

## Unit tests

`Smalltalk/Test` has a test class for every class in `Smalltalk/System`, in the same subfolders (`TestArray` tests `Array`, and so on), and a `Main.st` that runs them all:

```sh
$> ./bin/lst -o ./bin/test.img ./Smalltalk/System ./Smalltalk/Test
$> ./bin/lst ./bin/test.img
All 74 tests passed (361 asserts)
```

In VS Code, the launch configuration "lst: build image and run unit tests" does the same. A test class is a subclass of `Test`; its test methods are the methods whose name starts with `test`, and a test checks with `self assert: aBoolean`. `Test all` runs every test method of every subclass, each in its own process, and reports each failed assert and each error.

## Folders

| Folder | Contents |
|---|---|
| `src/` | the C++ sources of `lst`: the VM and the compiler |
| `Smalltalk/System/` | the system classes, one file per class: `Base`, `Collection`, `Magnitude`, `OS`, `Reflection` |
| `Smalltalk/Test/` | the unit tests, in the same subfolders |
| `Smalltalk/Examples/` | example programs, each with `build.sh` and `start.sh` |
| `bin/` | what the build makes: `lst` and the shell image `lst.img` |
| `cmake/` | the CMake script that bootstraps the image |

## Credits and license

Little Smalltalk was created by Timothy A. Budd of Oregon State University, starting in 1984; version 4.0 dates from about 1994. Andy Valencia and Kyle Hayes extended it (versions 4.5 to 4.7.1, [kyle-github/littlesmalltalk](https://github.com/kyle-github/littlesmalltalk)). Dr. Budd released his sources for any use, and Kyle Hayes relicensed the code under the 3-clause BSD license: see the file `LICENSE`.
