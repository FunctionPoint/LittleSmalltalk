#include "MethodCache.h"

/* flush dynamic methods when GC occurs */
void MethodCache::flush()
{
	for( int i = 0; i < cacheSize; i++ )
		cache[ i ].name = 0; /* force refill */
}

int64_t MethodCache::hits() const
{
	return cacheHit;
}

int64_t MethodCache::misses() const
{
	return cacheMiss;
}

int MethodCache::indexOf( Object* selector, Object* _class )
{
	return (int) ( ( ( (uintptr_t) selector ) + ( (uintptr_t) _class ) ) % (uintptr_t) cacheSize );
}

Object* MethodCache::find( Object* selector, Object* _class )
{
	Entry& entry = cache[ indexOf( selector, _class ) ];

	if( ( entry.name == selector ) && ( entry._class == _class ) ) {
		cacheHit++;
		return entry.method;
	}
	cacheMiss++;
	return nullptr;
}

void MethodCache::store( Object* selector, Object* _class, Object* method )
{
	Entry& entry = cache[ indexOf( selector, _class ) ];

	entry.name = selector;
	entry._class = _class;
	entry.method = method;
}
