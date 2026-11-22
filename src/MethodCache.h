#pragma once

/*
	MethodCache
	Cache of method lookups, for speeding up message sends: (selector,
	class) to method, hashed on the two pointers. Must be flushed when the
	garbage collector moves objects, and when methods change.
*/

#include <stdint.h>

class Object;

class MethodCache
{
public:
	static constexpr int cacheSize = 703;

private:
	struct Entry {
		Object* name;
		Object* _class;
		Object* method;
	};

	Entry cache[ cacheSize ] = {};
	int64_t cacheHit = 0;
	int64_t cacheMiss = 0;

public:
	Object* find( Object* selector, Object* _class );
	void store( Object* selector, Object* _class, Object* method );
	void flush();

	int64_t hits() const;
	int64_t misses() const;

private:
	static int indexOf( Object* selector, Object* _class );
};
