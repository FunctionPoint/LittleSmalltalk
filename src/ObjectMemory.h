#pragma once

/*
	ObjectMemory
	Memory management for the Little Smalltalk system: a variation on the
	Baker two-space algorithm. Objects are allocated downwards from the top
	of the current space; when it is full, everything reachable from the
	roots is copied to the other space. When that leaves less than a
	quarter of the space free (or not enough for the object), the dynamic
	space grows: both spaces are replaced by larger ones (twice the size,
	or more if one object needs it) and the live objects are copied into
	the new space.

	The static space (only for the CompiledImage that lst -o compiles) is not
	collected and its objects never move, so it grows differently: when its
	block is full, a new block is added (twice the size, or more for a large
	object), and the old blocks stay where they are.

	Roots: the root stack (pushRoot/popRoot, for the interpreter's
	temporaries) and the static roots (addStaticRoot: addresses of object
	pointers held outside object memory, like the SpecialObjects).

	Written by Tim Budd, budd@cs.orst.edu
	Relicensed under BSD 3-clause license per permission from Dr. Budd by
	Kyle Hayes. See LICENSE file.
*/

#include <stdint.h>
#include <functional>
#include <vector>

#include "Object.h"

class ObjectMemory
{
public:
	static constexpr int rootStackLimit = 2000;
	static constexpr int staticRootLimit = 200;
	static constexpr int defaultDynamicSize = 300000;
	static constexpr int defaultStaticSize = 300000;

private:
	/* static memory space */
	struct StaticBlock {
		Object* base;
		Object* top;
		Object* pointer;
	};
	std::vector<StaticBlock> staticBlocks;
	Object* staticPointer = nullptr;

	/* dynamic memory space */
	int spaceSize = 0;
	Object* spaceOne = nullptr;
	Object* spaceTwo = nullptr;
	bool inSpaceOne = true;

	Object* memoryBase = nullptr;
	Object* memoryPointer = nullptr;
	Object* memoryTop = nullptr;

	/* the space being collected */
	Object* oldBase = nullptr;
	Object* oldTop = nullptr;

	/* roots */
	Object* rootStack[ rootStackLimit ] = {};
	int rootTop = 0;
	Object** staticRoots[ staticRootLimit ] = {};
	int staticRootTop = 0;

	std::function<void()> afterCollect;

	/* statistics */
	int64_t gcCount = 0;
	int64_t gcTotalTime = 0;
	int64_t gcMaxTime = 0;
	int64_t gcTotalMemoryCopied = 0;
	int64_t gcMemoryMaxCopied = 0;

public:
	ObjectMemory();
	~ObjectMemory();
	ObjectMemory( const ObjectMemory& ) = delete;
	ObjectMemory& operator=( const ObjectMemory& ) = delete;

	void setAfterCollect( std::function<void()> callback );

	Object* allocate( int size );
	Object* allocateBytes( int size );

	Object* allocateStatic( int size );
	void releaseStatic();

	/* root stack: everything on it survives a collection (and may be moved) */
	void pushRoot( Object* object );
	Object* popRoot();
	Object* peekRoot( int depth = 0 ) const;

	void addStaticRoot( Object** objectPointer );

	void collect();
	void reserve( size_t byteCount );

	bool isDynamic( Object* object ) const;

	void exchangeObjects( Object* array1, Object* array2, int size );

	/* for the image reader */
	Object* pointer() const;
	Object* top() const;
	Object* unusedSpace() const;

	/* statistics */
	int64_t collections() const;
	int64_t totalTime() const;
	int64_t maxTime() const;
	int64_t totalCopied() const;
	int64_t maxCopied() const;

private:
	Object* collectAndAllocate( int size );
	void moveLiveObjects( Object* fromBase, int fromSize, Object* toBase, int toSize );
	void grow( int newSize );
	intptr_t freeWords() const;
	void addStaticBlock( int size );
	Object* gcMove( MObject* object );
	bool inNewSpace( const void* object ) const;
	bool inOldSpace( const void* object ) const;

	static void map( Object** objectPointer, Object* array1, Object* array2, int size );
	static void walk( Object* base, Object* top, Object* array1, Object* array2, int size );
};
