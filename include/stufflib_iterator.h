#ifndef _SL_ITERATOR_H_INCLUDED
#define _SL_ITERATOR_H_INCLUDED
#include <stdlib.h>
#include <string.h>

struct sl_iterator;

typedef void* sl_iterator_get_item(struct sl_iterator*);
typedef void sl_iterator_advance(struct sl_iterator*);
typedef bool sl_iterator_is_done(struct sl_iterator*);

struct sl_iterator {
  size_t index;
  size_t pos;
  void* data;
  sl_iterator_get_item* get_item;
  sl_iterator_advance* advance;
  sl_iterator_is_done* is_done;
};

#endif  // _SL_ITERATOR_H_INCLUDED
