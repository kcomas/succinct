
#include "error.h"

extern inline error *error_init(void);

extern inline void error_free(error *e);

extern inline void error_errno(error *const e);

extern inline void errno_print_exit(void);
