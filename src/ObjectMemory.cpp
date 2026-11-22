#include <stdlib.h>
#include <stddef.h>

#include "platform.h"
#include "Log.h"
#include "ObjectMemory.h"

/*
	allocate the memory areas
*/
ObjectMemory::ObjectMemory()
{
	spaceOne = (Object*) calloc( (size_t) defaultDynamicSize, sizeof( Object ) );
	spaceTwo = (Object*) calloc( (size_t) defaultDynamicSize, sizeof( Object ) );

	if( ( spaceOne == NULL ) || ( spaceTwo == NULL ) )
		Log::error( "gcinit(): not enough memory for object space allocations!" );

	addStaticBlock( defaultStaticSize );

	spaceSize = defaultDynamicSize;
	memoryBase = spaceOne;
	memoryPointer = memoryBase + spaceSize;
	memoryTop = memoryPointer;

	inSpaceOne = true;
}

ObjectMemory::~ObjectMemory()
{
	for( StaticBlock& block : staticBlocks )
		free( block.base );
	free( spaceOne );
	free( spaceTwo );
}

void ObjectMemory::setAfterCollect( std::function<void()> callback )
{
	afterCollect = std::move( callback );
}

/*
	FIXME - this is not portable.
*/
bool ObjectMemory::isDynamic( Object* object ) const
{
	return ( ( object >= spaceOne ) && ( object <= ( spaceOne + spaceSize ) ) ) ||
		   ( ( object >= spaceTwo ) && ( object <= ( spaceTwo + spaceSize ) ) );
}

bool ObjectMemory::inNewSpace( const void* object ) const
{
	return ( (intptr_t) object >= (intptr_t) memoryBase ) && ( (intptr_t) object <= (intptr_t) memoryTop );
}

bool ObjectMemory::inOldSpace( const void* object ) const
{
	return ( (intptr_t) object >= (intptr_t) oldBase ) && ( (intptr_t) object <= (intptr_t) oldTop );
}

/*
	gcMove is the heart of the garbage collection algorithm.
	It takes as argument a pointer to a value in the old space,
	and moves it, and everything it points to, into the new space
	The returned value is the address in the new space.
*/

Object* ObjectMemory::gcMove( MObject* object )
{
	MObject *oldAddress = object, *previousObject = 0, *newAddress = 0, *replacement = 0;
	int size;

	while( 1 ) {

		/*
		 * part 1.  Walking down the tree
		 * keep stacking objects to be moved until we find
		 * one that we can handle
		 */
		for( ;; ) {
			/*
			 * SmallInt's are not proper memory pointers,
			 * so catch them first.  Their "object pointer"
			 * value can be used as-is in the new space.
			 */
			if( SmallInt::is( oldAddress ) ) {
				replacement = oldAddress;
				oldAddress = previousObject;
				break;

				/*
				 * If we find a pointer in the current space
				 * to the new space (other than indirections) then
				 * something is very wrong
				 */
			} else if( inNewSpace( oldAddress ) )
				Log::error( "gcMove(): GC invariant failure -- old address points to new space %p", oldAddress );

			/* else see if not  in old space */
			else if( !inOldSpace( oldAddress ) ) {
				replacement = oldAddress;
				oldAddress = previousObject;
				break;

				/* else see if already forwarded */
			} else if( oldAddress->isGcDone() ) {
				if( oldAddress->isBinary() )
					/* reuse class element. */
					replacement = oldAddress->data[ 0 ];
				else {
					size = oldAddress->size();
					replacement = oldAddress->data[ size ];
				}
				oldAddress = previousObject;
				break;

				/* else see if binary object */
			} else if( oldAddress->isBinary() ) {
				int byteSize = oldAddress->size();
				size = Object::toWords( byteSize );
				memoryPointer = Object::wordsDown( memoryPointer, size + 2 );
				newAddress = (MObject*) memoryPointer;
				newAddress->setSize( byteSize );
				newAddress->setBinary();
				/* FIXME - use memcpy */
				while( size ) {
					newAddress->data[ size ] =
						oldAddress->data[ size ];
					size--;
				}
				oldAddress->setGcDone();
				newAddress->data[ 0 ] = previousObject;
				previousObject = oldAddress;
				oldAddress = oldAddress->data[ 0 ];
				previousObject->data[ 0 ] = newAddress;
				/* now go chase down class pointer */

				/* must be non-binary object */
			} else {
				size = oldAddress->size();
				memoryPointer = Object::wordsDown( memoryPointer, size + 2 );
				newAddress = (MObject*) memoryPointer;
				newAddress->setSize( size );
				oldAddress->setGcDone();
				newAddress->data[ size ] = previousObject;
				previousObject = oldAddress;
				oldAddress = oldAddress->data[ size ];
				previousObject->data[ size ] = newAddress;
			}
		}

		/*
		 * part 2.  Fix up pointers,
		 * move back up tree as long as possible
		 * old_address points to an object in the old space,
		 * which in turns points to an object in the new space,
		 * which holds a pointer that is now to be replaced.
		 * the value in replacement is the new value
		 */
		for( ;; ) {
			/* backed out entirely */
			if( oldAddress == 0 )
				return (Object*) replacement;

			/* case 1, binary or last value */
			if( oldAddress->isBinary() ||
				( oldAddress->size() == 0 ) ) {

				/* fix up class pointer */
				newAddress = oldAddress->data[ 0 ];
				previousObject = newAddress->data[ 0 ];
				newAddress->data[ 0 ] = replacement;
				oldAddress->data[ 0 ] = newAddress;
				replacement = newAddress;
				oldAddress = previousObject;
			} else {
				size = oldAddress->size();
				newAddress = oldAddress->data[ size ];
				previousObject = newAddress->data[ size ];
				newAddress->data[ size ] = replacement;
				size--;

				/*
				 * quick cheat for recovering zero fields
				 */
				while( size && ( oldAddress->data[ size ] == 0 ) )
					newAddress->data[ size-- ] = 0;

				oldAddress->setSize( size );
				oldAddress->setGcDone();
				newAddress->data[ size ] = previousObject;
				previousObject = oldAddress;
				oldAddress = oldAddress->data[ size ];
				previousObject->data[ size ] = newAddress;
				break; /* go track down this value */
			}
		}
	}

	/* make the compiler happy */
	return (Object*) 0;
}

void ObjectMemory::collect()
{
	/* change spaces, then copy */
	Object* fromBase = inSpaceOne ? spaceOne : spaceTwo;
	Object* toBase = inSpaceOne ? spaceTwo : spaceOne;

	inSpaceOne = !inSpaceOne;
	moveLiveObjects( fromBase, spaceSize, toBase, spaceSize );
}

void ObjectMemory::grow( int newSize )
{
	Object* newOne = (Object*) calloc( (size_t) newSize, sizeof( Object ) );
	Object* newTwo = (Object*) calloc( (size_t) newSize, sizeof( Object ) );

	if( ( newOne == NULL ) || ( newTwo == NULL ) )
		Log::error( "not enough memory to grow the object space to %d units!", newSize );

	/* the live objects are in the current space: copy them into the first new one */
	moveLiveObjects( inSpaceOne ? spaceOne : spaceTwo, spaceSize, newOne, newSize );

	free( spaceOne );
	free( spaceTwo );
	spaceOne = newOne;
	spaceTwo = newTwo;
	spaceSize = newSize;
	inSpaceOne = true;
}

void ObjectMemory::reserve( size_t byteCount )
{
	size_t units = ( byteCount + sizeof( Object ) - 1 ) / sizeof( Object );

	if( memoryPointer != memoryTop )
		Log::error( "reserve(): the object space is not empty!" );
	if( units <= (size_t) spaceSize )
		return;
	if( units > (size_t) INT32_MAX )
		Log::error( "reserve(): %zu bytes is too much!", byteCount );
	/* nothing to move: just replace the empty spaces */
	grow( (int) units );
}

intptr_t ObjectMemory::freeWords() const
{
	return ( (char*) memoryPointer - (char*) memoryBase ) / BytesPerWord;
}

void ObjectMemory::moveLiveObjects( Object* fromBase, int fromSize, Object* toBase, int toSize )
{
	int64_t start = lstTimeUsec();
	int64_t end = 0;

	memoryBase = toBase;
	memoryPointer = memoryTop = memoryBase + toSize;
	oldBase = fromBase;
	oldTop = oldBase + fromSize;

	/* then do the collection */
	for( int i = 0; i < rootTop; i++ )
		rootStack[ i ] = gcMove( (MObject*) rootStack[ i ] );
	for( int i = 0; i < staticRootTop; i++ )
		( *staticRoots[ i ] ) = gcMove( (MObject*) *staticRoots[ i ] );

	if( afterCollect )
		afterCollect();

	/* FIXME - pointer comparisons are NOT portable. */
	gcTotalMemoryCopied += ( (char*) memoryTop - (char*) memoryPointer );
	if( ( (char*) memoryTop - (char*) memoryPointer ) > gcMemoryMaxCopied )
		gcMemoryMaxCopied = ( (char*) memoryTop - (char*) memoryPointer );

	end = lstTimeUsec();

	/* calculate stats about the GC runs. */
	gcCount++;
	gcTotalTime += ( end - start );

	if( gcMaxTime < ( end - start ) )
		gcMaxTime = ( end - start );
}

/*
	garbage collection entry point
*/
Object* ObjectMemory::collectAndAllocate( int size )
{
	/* force a GC */
	collect();

	/* grow if the object does not fit, or if less than a quarter of the space is free */
	intptr_t spaceWords = (intptr_t) spaceSize * (intptr_t) ( sizeof( Object ) / BytesPerWord );
	intptr_t neededWords = (intptr_t) size + 2;
	if( freeWords() < neededWords || freeWords() < spaceWords / 4 ) {
		intptr_t usedWords = spaceWords - freeWords();
		/* twice the size, and at least room for the live objects and the new one with half the space free */
		intptr_t newWords = spaceWords * 2;
		if( newWords < ( usedWords + neededWords ) * 2 )
			newWords = ( usedWords + neededWords ) * 2;
		intptr_t newUnits = ( newWords + (intptr_t) ( sizeof( Object ) / BytesPerWord ) - 1 ) / (intptr_t) ( sizeof( Object ) / BytesPerWord );
		if( newUnits > INT32_MAX )
			Log::error( "cannot grow the object space for an object of size %d!", size );
		grow( (int) newUnits );
	}

	/* then allocate */
	memoryPointer = Object::wordsDown( memoryPointer, size + 2 );
	if( (intptr_t) memoryPointer < (intptr_t) memoryBase )
		Log::error( "insufficient memory after garbage collection when allocating object of size %d!", size );
	memoryPointer->setSize( size );

	return ( memoryPointer );
}

Object* ObjectMemory::allocateBytes( int size )
{
	int trueSize = Object::toWords( size );
	Object* result = allocate( trueSize );
	result->setSize( size );
	result->setBinary();
	return result;
}

void ObjectMemory::addStaticBlock( int size )
{
	Object* base = (Object*) calloc( (size_t) size, sizeof( Object ) );

	if( base == NULL )
		Log::error( "not enough memory for a static block of %d units!", size );
	/* the block that was being used is full: remember where its objects start */
	if( !staticBlocks.empty() )
		staticBlocks.back().pointer = staticPointer;
	staticBlocks.push_back( { base, base + size, base + size } );
	staticPointer = base + size;
}

Object* ObjectMemory::allocateStatic( int size )
{
	Object* result = Object::wordsDown( staticPointer, size + 2 );

	if( (intptr_t) result < (intptr_t) staticBlocks.back().base ) {
		/* a new block: twice the size of the last one, or enough for this object */
		intptr_t lastSize = staticBlocks.back().top - staticBlocks.back().base;
		intptr_t neededUnits = ( ( (intptr_t) size + 2 ) * BytesPerWord + (intptr_t) sizeof( Object ) - 1 ) / (intptr_t) sizeof( Object );
		intptr_t newSize = lastSize * 2 > neededUnits ? lastSize * 2 : neededUnits;
		if( newSize > INT32_MAX )
			Log::error( "cannot add a static block for an object of size %d!", size );
		addStaticBlock( (int) newSize );
		result = Object::wordsDown( staticPointer, size + 2 );
	}
	staticPointer = result;
	staticPointer->setSize( size );
	return staticPointer;
}

void ObjectMemory::releaseStatic()
{
	/* keep only the first block, empty */
	for( size_t i = 1; i < staticBlocks.size(); i++ )
		free( staticBlocks[ i ].base );
	staticBlocks.resize( 1 );
	staticPointer = staticBlocks[ 0 ].top;
	staticBlocks[ 0 ].pointer = staticPointer;
}

/*
 * addStaticRoot()
 *  Add another object root off a static object
 *
 * Static objects, in general, do not get garbage collected.  When
 * a static object is discovered adding a reference to a non-static
 * object, we link on the reference to our staticRoot table so we can
 * give it proper treatment during garbage collection.
 */
void ObjectMemory::addStaticRoot( Object** objectPointer )
{
	for( int i = 0; i < staticRootTop; ++i )
		if( objectPointer == staticRoots[ i ] )
			return;
	if( staticRootTop >= staticRootLimit )
		Log::error( "addStaticRoot(): too many static references (max %d) to store object %p!", staticRootLimit, objectPointer );
	staticRoots[ staticRootTop++ ] = objectPointer;
}

/*
 * map()
 *  Fix an OOP if needed, based on values to be exchanged
 *
 * walk()
 *  Traverse an object space
 */
void ObjectMemory::map( Object** objectPointer, Object* array1, Object* array2, int size )
{
	Object* object = *objectPointer;

	for( int i = 0; i < size; ++i ) {
		if( array1->data[ i ] == object ) {
			*objectPointer = array2->data[ i ];
			return;
		}
		if( array2->data[ i ] == object ) {
			*objectPointer = array1->data[ i ];
			return;
		}
	}
}
void ObjectMemory::walk( Object* base, Object* top,
						 Object* array1, Object* array2, int size )
{
	Object* nextObject;

	for( Object* object = base; object < top; object = nextObject ) {
		/*
		 * Re-map the class pointer, in case that's the
		 * object which has been remapped.
		 */
		map( &object->objectClass, array1, array2, size );

		/*
		 * Skip our argument arrays, since otherwise things
		 * get rather circular.
		 */
		int objectSize = object->size();
		if( ( object == array1 ) || ( object == array2 ) ) {
			nextObject = Object::wordsUp( object, objectSize + 2 );
			continue;
		}

		/*
		 * Don't have to worry about instance variables
		 * if it's a binary format.
		 */
		if( object->isBinary() ) {
			/*
			 * Skip size/class, and enough words to
			 * contain the binary bytes.
			 */
			int trueSize = Object::toWords( objectSize );
			nextObject = Object::wordsUp( object, trueSize + 2 );
			continue;
		}

		/*
		 * For each instance variable slot, fix up the pointer
		 * if needed.
		 */
		for( int i = 0; i < objectSize; ++i )
			map( &object->data[ i ], array1, array2, size );

		/*
		 * Walk past this object
		 */
		nextObject = Object::wordsUp( object, objectSize + 2 );
	}
}

/*
 * exchangeObjects()
 *  Bulk exchange of object identities
 *
 * For each index to array1/array2, all references in current object
 * memory are modified so that references to the object in array1[]
 * become references to the corresponding object in array2[].  References
 * to the object in array2[] similarly become references to the
 * object in array1[].
 */
void ObjectMemory::exchangeObjects( Object* array1, Object* array2, int size )
{
	/*
	 * Convert our memory spaces
	 */
	walk( memoryPointer, memoryTop, array1, array2, size );
	for( size_t block = 0; block < staticBlocks.size(); block++ ) {
		/* the objects of a block: from where allocation stopped to its top */
		Object* from = ( block + 1 == staticBlocks.size() ) ? staticPointer : staticBlocks[ block ].pointer;
		walk( from, staticBlocks[ block ].top, array1, array2, size );
	}

	/*
	 * Fix up the root pointers, too
	 */
	for( int i = 0; i < rootTop; i++ )
		map( &rootStack[ i ], array1, array2, size );
	for( int i = 0; i < staticRootTop; i++ )
		map( staticRoots[ i ], array1, array2, size );
}

Object* ObjectMemory::pointer() const
{
	return memoryPointer;
}

Object* ObjectMemory::top() const
{
	return memoryTop;
}

Object* ObjectMemory::unusedSpace() const
{
	return inSpaceOne ? spaceTwo : spaceOne;
}

int64_t ObjectMemory::collections() const
{
	return gcCount;
}

int64_t ObjectMemory::totalTime() const
{
	return gcTotalTime;
}

int64_t ObjectMemory::maxTime() const
{
	return gcMaxTime;
}

int64_t ObjectMemory::totalCopied() const
{
	return gcTotalMemoryCopied;
}

int64_t ObjectMemory::maxCopied() const
{
	return gcMemoryMaxCopied;
}

Object* ObjectMemory::allocate( int size )
{
	memoryPointer = Object::wordsDown( memoryPointer, size + 2 );
	if( (intptr_t) memoryPointer < (intptr_t) memoryBase )
		return collectAndAllocate( size );
	memoryPointer->setSize( size );
	return memoryPointer;
}

void ObjectMemory::pushRoot( Object* object )
{
	rootStack[ rootTop++ ] = object;
}

Object* ObjectMemory::popRoot()
{
	return rootStack[ --rootTop ];
}

Object* ObjectMemory::peekRoot( int depth ) const
{
	return rootStack[ rootTop - 1 - depth ];
}
