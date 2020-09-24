
#include "token.h"

extern inline void token_init(token *const t);

static inline char get_char(token *const t, const string *s) {
    if (t->end_idx >= s->len) return '\n';
    return s->buffer[t->end_idx];
}

static inline void next_char_update(token *const t) {
    t->end_idx++;
    t->char_no++;
}

static inline void newline_update(token *const t) {
    t->end_idx++;
    t->char_no = 1;
    t->line_no++;
}

static void remove_spaces(token *const t, const string *s) {
    for (;;) {
        char c = get_char(t, s);
        if (c == ' ') next_char_update(t);
        else if (c == '\n') newline_update(t);
        else break;
    }
}

token_status token_next(token *const t, const string *s) {
    remove_spaces(t, s);
    t->start_idx = t->end_idx;
    char c = get_char(t, s);
    printf("%c\n", c);
    return 0;
}
