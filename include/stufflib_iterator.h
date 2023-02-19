#ifndef _STUFFLIB_ITERATOR_H_INCLUDED
#define _STUFFLIB_ITERATOR_H_INCLUDED
#include <stdlib.h>
#include <string.h>

typedef struct stufflib_iterator stufflib_iterator;
struct stufflib_iterator;

typedef void* stufflib_iterator_get(stufflib_iterator*, void*);
typedef void stufflib_iterator_advance(stufflib_iterator*);
typedef bool stufflib_iterator_end(stufflib_iterator*);

struct stufflib_iterator {
  size_t index;
  size_t pos;
  void* begin;
  stufflib_iterator_get* get;
  stufflib_iterator_advance* advance;
  stufflib_iterator_end* end;
};

#endif  // _STUFFLIB_ITERATOR_H_INCLUDED
