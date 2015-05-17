#include "list.h"
#include "gdsl/gdsl_list.h"

long int compare_func(const gdsl_element_t elem, void* value) {
  if (elem == value) return 0;
  if (elem < value) return -1;
  else return 1;
}

List list_alloc() {
  return gdsl_list_alloc("", NULL, NULL);
}

void* list_first(List list) {
  return (void*)gdsl_list_get_head(list);
}

void* list_last(List list) {
  return (void*)gdsl_list_get_tail(list);
}

void* list_get(size_t pos, List list) {
  return (void*)gdsl_list_search_by_position(list, pos+1);
}

int list_insert(size_t pos, void* elem, List list) {
  if (pos < gdsl_list_get_size(list)) return -1;
  gdsl_list_cursor_t cursor = gdsl_list_cursor_alloc( list );
  gdsl_list_cursor_move_to_position ( cursor, pos+1 );
  gdsl_list_cursor_insert_before(cursor, elem);
  gdsl_list_cursor_free(cursor);
  return 0;
}

int list_remove_pos(size_t pos, List list) {
  gdsl_element_t elem = gdsl_list_search_by_position(list, pos+1);
  return list_remove_elem(elem, list);
}

int list_remove_elem(void* elem, List list) {
  if (!elem) return -1;
  gdsl_list_delete(list, compare_func, elem);
  return 0;
}

void list_add(void* elem, List list) {
  gdsl_list_insert_tail(list, elem);
}

void list_clear(List list) {
  gdsl_list_flush(list);
}

size_t list_size(List list) {
  return gdsl_list_get_size(list);
}

void list_free(List list) {
  gdsl_list_free(list);
}
