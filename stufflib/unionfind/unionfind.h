#ifndef SL_UNIONFIND_H_INCLUDED
#define SL_UNIONFIND_H_INCLUDED

#include <stddef.h>

#include <stufflib/context/context.h>

struct sl_unionfind {
  size_t count;
  size_t* parents;
};

struct sl_unionfind*
sl_unionfind_init(struct sl_context ctx[static 1], struct sl_unionfind uf[static 1], size_t count);
void sl_unionfind_destroy(struct sl_unionfind uf[static 1]);
size_t sl_unionfind_find_root(const struct sl_unionfind uf[const static 1], size_t index);
void sl_unionfind_set_root(const struct sl_unionfind uf[const static 1], size_t begin, size_t root);
void sl_unionfind_union(const struct sl_unionfind uf[const static 1], size_t lhs, size_t rhs);

#endif  // SL_UNIONFIND_H_INCLUDED
