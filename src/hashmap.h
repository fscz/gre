#ifndef gre_hashmap_h
#define gre_hashmap_h

#include <stdlib.h>

typedef void* HashMap;

HashMap hashmap_alloc();
HashMap hashmap_alloc_size(const size_t size);
void hashmap_free(HashMap map);
void hashmap_insert(const char* key, void* value, HashMap map);
void hashmap_delete(const char* key, HashMap map);
void* hashmap_find(const char* key, HashMap map);
size_t hashmap_size(HashMap map);


#endif
