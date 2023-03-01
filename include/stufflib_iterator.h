#ifndef _SL_ITERATOR_H_INCLUDED
#define _SL_ITERATOR_H_INCLUDED
#include <stdlib.h>
#include <string.h>

typedef struct sl_iterator sl_iterator;
struct sl_iterator;

typedef void* sl_iterator_get_item(sl_iterator*);
typedef void sl_iterator_advance(sl_iterator*);
typedef bool sl_iterator_is_done(sl_iterator*);

struct sl_iterator {
  size_t index;
  size_t pos;
  void* data;
  sl_iterator_get_item* get_item;
  sl_iterator_advance* advance;
  sl_iterator_is_done* is_done;
};

#endif  // _SL_ITERATOR_H_INCLUDED
