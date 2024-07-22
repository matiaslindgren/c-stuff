#ifndef SL_ITERATOR_H_INCLUDED
#define SL_ITERATOR_H_INCLUDED

struct sl_iterator;

typedef void* sl_iterator_get(struct sl_iterator*);
typedef void sl_iterator_advance(struct sl_iterator*);
typedef bool sl_iterator_is_done(struct sl_iterator*);

struct sl_iterator {
  size_t index;
  size_t pos;
  void* data;
};

#endif  // SL_ITERATOR_H_INCLUDED
