#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "hashmap.h"
#include "gdsl/gdsl_hash.h"

#define KEY_SIZE 20
#define DEFAULT_MAP_SIZE 20


typedef struct {
  char* key;
  void* value;
} Element;

static const char* key_func(gdsl_element_t e) {
  Element* elem = (Element*)e;
  return elem->key;
}

static void free_func(gdsl_element_t e) {
  Element* elem = (Element*)e;
  free (elem->key);
  free (elem);
}

HashMap hashmap_alloc() {
  return hashmap_alloc_size(DEFAULT_MAP_SIZE);
}

HashMap hashmap_alloc_size(size_t size) {
  return gdsl_hash_alloc("", NULL, free_func, key_func, NULL, size);
}

void hashmap_free(HashMap map) {
  gdsl_hash_free ( (gdsl_hash_t)map );
}

void* hashmap_find(const char* key, HashMap map) {
  Element* elem = gdsl_hash_search(map, key);
  if (elem) return elem->value;
  else return NULL;
}

void hashmap_insert(const char* key, void* value, HashMap map) {  
  Element* element = malloc(sizeof(Element));
  element->key = strdup(key);
  element->value = value;

  gdsl_hash_insert(map, element);
}

size_t hashmap_size(HashMap map) {
  return gdsl_hash_get_size (map);
}

void hashmap_delete(const char* key, HashMap map) {
  gdsl_hash_remove(map, key);
}
