#ifndef SL_ARGS_H_INCLUDED
#define SL_ARGS_H_INCLUDED

bool sl_args_is_flag(const struct sl_args args[const static 1], const int i);

int sl_args_count_positional(const struct sl_args args[const static 1]);

int sl_args_count_optional(const struct sl_args args[const static 1]);

char* sl_args_get_positional(const struct sl_args args[const static 1],
                             const int pos);

char* sl_args_find_optional(const struct sl_args args[const static 1],
                            const char arg[const static 1]);

bool sl_args_parse_flag(const struct sl_args args[const static 1],
                        const char arg[const static 1]);

unsigned long long sl_args_parse_ull(const struct sl_args args[const static 1],
                                     const char arg[const static 1],
                                     const int base);

#endif  // SL_ARGS_H_INCLUDED
