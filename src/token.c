
#include "token.h"

extern inline token *token_init(void);

extern inline void token_free(token *t);

extern inline size_t token_len(const token *const t);

const char *token_type_string(token_type type) {
    static const char *types[] = {
        "_START_TOKENS",
        "UNKNOWN",
        "VAR",
        "INT",
        "CHAR",
        "STRING",
        "U64",
        "LBRACE",
        "RBRACE",
        "LBRACKET",
        "RBRACKET",
        "LPARENS",
        "RPARENS",
        "ASSIGN",
        "DEFINE",
        "ADD",
        "SUB",
        "MUL",
        "WRITE",
        "NEWLINE",
        "SEPRATOR",
        "COND",
        "EQUAL",
        "LESS",
        "LESSEQUAL",
        "AND",
        "_MAX_TOKENS"
    };
    return type > TOKEN_PFX(_START_TOKENS) && type < TOKEN_PFX(_END_TOKENS) ? types[type] : "TOKEN_TYPE_NOT_FOUND";
};

void token_print_json(const token *const t, const string *const s) {
    printf("{\"type\":\"%s\",\"line\":%lu,\"char\":%lu,\"len\":%lu,\"str\":\"", token_type_string(t->type), t->line_no, t->char_no, token_len(t));
    if (t->type == TOKEN_PFX(NEWLINE)) {
        printf("\\n");
    } else {
        for(size_t i = t->start_idx; i <= t->end_idx; i++) {
            if (s->buffer[i] == '"') putchar('\\');
            putchar(s->buffer[i]);
        }
    }
    printf("\"}");
}

extern inline token *token_copy(token *const dest, const token *const src);

static void next_char_update(token *const t) {
    t->end_idx++;
    t->char_no++;
}

static char token_char_lookup(const token *const t,  const string *const s, size_t n) {
    if (t->start_idx + n >= s->len) return '\0';
    return s->buffer[t->start_idx + n];
}

static char peek_char_n(const token *const t,  const string *const s, size_t n) {
    if (t->end_idx + n >= s->len) return '\0';
    return s->buffer[t->end_idx + n];
}

static char peek_char(const token *const t, const string *const s) {
    return peek_char_n(t, s, 1);
}

static char get_char(const token *const t, const string *const s) {
    return peek_char_n(t, s, 0);
}

static void newline_update(token *const t) {
    t->char_no = 1;
    t->line_no++;
}

static void remove_spaces(token *const t, const string *const s) {
    for (;;) {
        char c = get_char(t, s);
        if (c == ' ' || c == '\t') next_char_update(t);
        else break;
    }
    t->start_idx = t->end_idx;
}

static token_status found_token(token *const t, token_type type) {
    t->type = type;
    return TOKEN_STATUS_PFX(SOME);
}

static token_status parse_var(token* const t, const string *const s) {
    // enter at current letter char if the next char is not letternum dont update position
    char peek;
    while (isalnum(peek = peek_char(t, s))) next_char_update(t);
    // check for type
    if (token_len(t) <= 3) {
        switch (token_char_lookup(t, s , 0)) {
            case 'u':
                switch (token_char_lookup(t, s, 1)) {
                    case '6':
                        if (token_char_lookup(t, s, 2)) return found_token(t, TOKEN_PFX(U64));
                        break;
                }
                break;
            default:
                break;
        }
    }
    return found_token(t, TOKEN_PFX(VAR));
}

static token_status parse_num(token* const t, const string *const s) {
    // TODO  floats
    char peek;
    while (isdigit(peek = peek_char(t, s))) next_char_update(t);
    return found_token(t, TOKEN_PFX(INT));
}

static token_status parse_string(token* const t, const string *const s) {
    static const size_t max_inline_string_size = 1024;
    // we are on first "
    char peek = peek_char(t, s);
    if (peek == '"') {
        // empty string
        next_char_update(t);
        return found_token(t, TOKEN_PFX(STRING));
    }
    size_t pos = 0;
    while ((peek = peek_char(t, s)) != '"') {
        next_char_update(t);
        if (++pos >= max_inline_string_size) return TOKEN_STATUS_PFX(EXCEDED_MAX_STRING_LEN);
    }
    next_char_update(t);
    // char if 3 chars for char of 4 chars for escape char
    if (token_len(t) == 3 || (token_len(t) == 4 && s->buffer[t->start_idx + 1] == '\\')) t->type = TOKEN_PFX(CHAR);
    else t->type = TOKEN_PFX(STRING);
    return TOKEN_STATUS_PFX(SOME);
}

static bool char_lookup_one(token *const t, const string *const s, char cmp) {
    if (peek_char(t, s) == cmp) {
        next_char_update(t);
        return true;
    }
    return false;
}

token_status token_next(token *const t, const string *const s) {
    t->type = TOKEN_PFX(UNKNOWN);
    if (t->end_idx == 0) if (get_char(t, s) != '\n') return TOKEN_STATUS_PFX(FILE_MUST_START_NEWLINE);
    next_char_update(t);
    remove_spaces(t, s);
    char c = get_char(t, s);
    if (c == '\0') return TOKEN_STATUS_PFX(NONE);
    if (isalpha(c)) return parse_var(t, s);
    if (isdigit(c)) return parse_num(t, s);
    if (c == '"') return parse_string(t, s);
    switch (c) {
        case '{': return found_token(t, TOKEN_PFX(LBRACE));
        case '}': return found_token(t, TOKEN_PFX(RBRACE));
        case '[': return found_token(t, TOKEN_PFX(LBRACKET));
        case ']': return found_token(t, TOKEN_PFX(RBRACKET));
        case '(': return found_token(t, TOKEN_PFX(LPARENS));
        case ')': return found_token(t, TOKEN_PFX(RPARENS));
        case ':':
            if (char_lookup_one(t, s, ':'))
                return found_token(t, TOKEN_PFX(DEFINE));
            else
                return found_token(t, TOKEN_PFX(ASSIGN));
        case '+': return found_token(t, TOKEN_PFX(ADD));
        case '-': return found_token(t, TOKEN_PFX(SUB));
        case '*': return found_token(t, TOKEN_PFX(MUL));
        case '\n':
                  newline_update(t);
                  return found_token(t, TOKEN_PFX(NEWLINE));
        case ';': return found_token(t, TOKEN_PFX(SEPRATOR));
        case '?': return found_token(t, TOKEN_PFX(COND));
        case '=': return found_token(t, TOKEN_PFX(EQUAL));
        case '<':
            if (char_lookup_one(t, s, '='))
                return found_token(t, TOKEN_PFX(LESSEQUAL));
            else if (char_lookup_one(t, s, '&'))
                return found_token(t, TOKEN_PFX(WRITE));
            else
                return found_token(t, TOKEN_PFX(LESS));
        case '&': return found_token(t, TOKEN_PFX(AND));
    }
    return TOKEN_STATUS_PFX(NONE);
}
