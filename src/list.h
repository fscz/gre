#ifndef gre_list_h
#define gre_list_h

#include <stdlib.h>

typedef void* List;

List list_alloc();
void* list_first(List list);
void* list_last(List list);
void* list_get(size_t pos, List list);
int list_insert(size_t pos, void* elem, List list);
int list_remove_pos(size_t pos, List list);
int list_remove_elem(void* elem, List list);
void list_add(void* elem, List list);
void list_clear(List list);
size_t list_size(List list);
void list_free(List list);


#endif
