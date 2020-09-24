
#include "token.h"

extern inline void token_init(token *const t);

const char *token_type_string(token_type type) {
    static const char* types[] = {
        "NONE",
        "VAR",
        "INT",
        "CHAR",
        "U64",
        "LBRACE",
        "RBRACE",
        "LBRACKET",
        "RBRACKET",
        "LPARENS",
        "RPARENS",
        "ASSIGN",
        "DEFINE",
        "SUB",
        "MUL",
        "WRITE",
        "NEWLINE",
        "SEPRATOR",
        "IF",
        "EQUAL",
        "LESSEQUAL",
        "_MAX_TOKENS"
    };
    return type < TOKEN_PFX(_MAX_TOKENS) ? types[type] : "UNKNOWN";
};

extern inline void token_print(const token *const t, const string *const s);

static char get_char(token *const t, const string *const s) {
    if (t->end_idx >= s->len) return '\0';
    return s->buffer[t->end_idx];
}

static void next_char_update(token *const t) {
    t->end_idx++;
    t->char_no++;
}

static char peek_char(const token *const t, const string *const s) {
    if (t->end_idx + 1 >= s->len) return '\0';
    return s->buffer[t->end_idx + 1];
}

static void newline_update(token *const t) {
    t->end_idx++;
    t->char_no = 1;
    t->line_no++;
}

static void remove_spaces(token *const t, const string *s) {
    t->start_idx = t->end_idx;
    for (;;) {
        char c = get_char(t, s);
        if (c == ' ' || c == '\t') next_char_update(t);
        else if (c == '\n') newline_update(t);
        else break;
    }
}

static token_status parse_var(token* const t, const string *const s) {
    // enter at current letter char if the next char is not letternum dont update position
    char peek;
    while (isalnum(peek = peek_char(t, s))) next_char_update(t);
    // TODO check for type
    t->type = TOKEN_PFX(VAR);
    return TOKEN_STATUS_PFX(SOME);
}

static token_status parse_num(token* const t, const string *const s) {
    // TODO  floats
    char peek;
    while (isdigit(peek = peek_char(t, s))) next_char_update(t);
    t->type = TOKEN_PFX(INT);
    return TOKEN_STATUS_PFX(SOME);
}

token_status token_next(token *const t, const string *const s) {
    remove_spaces(t, s);
    // on a non \s char if the start and end are same
    if (t->start_idx == t->end_idx) t->end_idx++;
    t->start_idx = t->end_idx;
    char c = get_char(t, s);
    if (c == '\0') return TOKEN_STATUS_PFX(NONE);
    if (isalpha(c)) return parse_var(t, s);
    if (isdigit(c)) return parse_num(t, s);
    switch (c) {

    }
    return TOKEN_STATUS_PFX(SOME);
}
