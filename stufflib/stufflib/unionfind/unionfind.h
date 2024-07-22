#ifndef SL_UNIONFIND_H_INCLUDED
#define SL_UNIONFIND_H_INCLUDED

struct sl_unionfind {
  size_t count;
  size_t* parents;
};

struct sl_unionfind* sl_unionfind_init(struct sl_unionfind uf[static 1],
                                       const size_t count);

void sl_unionfind_destroy(struct sl_unionfind uf[static 1]);

size_t sl_unionfind_find_root(const struct sl_unionfind uf[const static 1],
                              size_t index);

void sl_unionfind_set_root(const struct sl_unionfind uf[const static 1],
                           const size_t begin,
                           const size_t root);

void sl_unionfind_union(const struct sl_unionfind uf[const static 1],
                        const size_t lhs,
                        const size_t rhs);

#endif  // SL_UNIONFIND_H_INCLUDED
