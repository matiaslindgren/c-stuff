#ifndef _STUFFLIB_ITERATOR_H_INCLUDED
#define _STUFFLIB_ITERATOR_H_INCLUDED
#include <stdlib.h>
#include <string.h>

typedef struct stufflib_iterator stufflib_iterator;
struct stufflib_iterator;

typedef void* stufflib_iterator_get_item(stufflib_iterator*);
typedef void stufflib_iterator_advance(stufflib_iterator*);
typedef bool stufflib_iterator_is_done(stufflib_iterator*);

struct stufflib_iterator {
  size_t index;
  size_t pos;
  void* data;
  stufflib_iterator_get_item* get_item;
  stufflib_iterator_advance* advance;
  stufflib_iterator_is_done* is_done;
};

#endif  // _STUFFLIB_ITERATOR_H_INCLUDED
