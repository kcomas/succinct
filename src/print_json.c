
#include "print_json.h"

static void print_token_string(const token *const t, const string *const s) {
    for(size_t i = t->start_idx; i <= t->end_idx; i++) {
        if (s->buffer[i] == '\n') {
            printf("\\n");
        } else {
            if (s->buffer[i] == '"') putchar('\\');
            putchar(s->buffer[i]);
        }
    }
}

void token_print_json(const token *const t, const string *const s) {
    printf("{\"type\":\"%s\",\"line\":%lu,\"char\":%lu,\"len\":%lu,\"str\":\"", token_type_string(t->type), t->line_no, t->char_no, token_len(t));
    switch (t->type) {
        case TOKEN_PFX(COMMENT):
            print_token_string(t, s);
            break;
        case TOKEN_PFX(NEWLINE):
            printf("\\n");
            break;
        default:
            print_token_string(t, s);
            break;
    }
    printf("\"}");
}

void symbol_table_bucket_print_json(const symbol_table_bucket *const b) {
    printf("{\"symbol_table_type\":\"%s\",\"symbol_idx\":%lu,\"size_len\":%lu,", symbol_table_type_string(b->table_type), b->symbol_idx, b->size_len);
    if (b->table_type == SYMBOL_PFX(KEY)) printf("\"key_idx\":%lu,", b->idx.key);
    else printf("\"stack_idx\":%lu", b->idx.stack);
    printf(",\"var_type\":");
    var_type_print_json(b->type);
    printf(",\"symbol\":\"%s\"}", b->symbol);
}

void symbol_table_print_json(const symbol_table *const table) {
    printf("{\"size\":%lu,\"symbol_counter\":%lu,\"buckets\":[", table->size, table->symbol_counter);
    for (size_t i = 0; i < table->size; i++) {
        symbol_table_bucket *b = table->buckets[i];
        while (b != NULL) {
            symbol_table_bucket_print_json(b);
            // only print comma if next is not null
            if (b->next != NULL) putchar(',');
            b = b->next;
        }
    }
    printf("]}");
}

void var_type_print_json(const var_type *const t) {
    printf("{\"header\":\"%s\",\"body\":{", var_type_header_string(t->header));
    switch (t->header) {
        case VAR_PFX(FN):
            printf("\"num_args\":%lu,\"num_locals\":%lu,\"return_type\":", t->body.fn->num_args, t->body.fn->num_locals);
            var_type_print_json(t->body.fn->return_type);
            printf(",\"symbol_table\":");
            symbol_table_print_json(t->body.fn->symbols);
            break;
        default:
            break;
    }
    printf("}}");
}

void ast_vec_node_print_json(const ast_vec_node *const vec, const string *const s) {
    printf("{\"num_items\":%lu,\"type\":", vec->num_items);
    if (vec->type != NULL) var_type_print_json(vec->type);
    else printf("null");
    putchar(',');
    printf("\"items\":");
    ast_node_link_print_json(vec->items_head, s);
    putchar('}');
}

void ast_fn_node_print_json(const ast_fn_node *const fn, const string *const s) {
    printf("{\"type\":");
    var_type_print_json(fn->type);
    /*
    printf(",\"parent\":");
    if (fn->parent != NULL) ast_fn_node_print_json(fn->parent, s);
    else printf("null");
    */
    printf(",\"body\":");
    ast_node_link_print_json(fn->body_head, s);
    putchar('}');
}

void ast_call_node_print_json(const ast_call_node *const c, const string *const s) {
    printf("{\"num_args\":%lu,\"func\":", c->num_args);
    ast_node_print_json(c->func, s);
    printf(",\"args\":[");
    for (size_t i = 0; i < c->num_args; i++) {
        ast_node_print_json(c->args[i], s);
        if (i < c->num_args - 1) putchar(',');
    }
    printf("]}");
}

void ast_if_node_print_json(const ast_if_node *const if_node, const string *const s) {
    printf("{\"return_type\":");
    var_type_print_json(if_node->return_type);
    printf(",\"conds\":[");
    ast_if_cond *head = if_node->conds_head;
    while (head != NULL) {
        printf("{\"cond\":");
        ast_node_print_json(head->cond, s);
        printf(",\"body\":");
        ast_node_link_print_json(head->body_head, s);
        putchar('}');
        if (head->next != NULL) putchar(',');
        head = head->next;
    }
    printf("],\"else\":");
    ast_node_link_print_json(if_node->else_head, s);
    putchar('}');
}

void ast_node_print_json(const ast_node *const node, const string *const s) {
    printf("{\"type\":\"%s\",\"data\":", ast_type_string(node->type));
    switch (node->type) {
        case AST_PFX(VAR):
            symbol_table_bucket_print_json(node->data.var);
            break;
        case AST_PFX(INT):
        case AST_PFX(CHAR):
            printf("null");
            break;
        case AST_PFX(VEC):
            ast_vec_node_print_json(node->data.vec, s);
            break;
        case AST_PFX(FN):
            ast_fn_node_print_json(node->data.fn, s);
            break;
        case AST_PFX(CALL):
            ast_call_node_print_json(node->data.call, s);
            break;
        case AST_PFX(IF):
            ast_if_node_print_json(node->data.ifn, s);
            break;
        default:
            if (is_op(node)) {
                // op node
                printf("{\"return_type\":");
                var_type_print_json(node->data.op->return_type);
                printf(",\"left\":");
                ast_node_print_json(node->data.op->left, s);
                printf(",\"right\":");
                ast_node_print_json(node->data.op->right, s);
                putchar('}');
            } else {
                printf("{\"error\":\"UNKNOWN_NODE\"}");
            }
            break;
    }
    putchar(',');
    printf("\"token\":");
    token_print_json(node->t, s);
    putchar('}');
}

void ast_node_link_print_json(ast_node_link *head, const string *const s) {
    // print all links
    putchar('[');
    while (head != NULL) {
        if (head->node) ast_node_print_json(head->node, s);
        if (head->next != NULL && head->next->node) putchar(',');
        head = head->next;
    }
    putchar(']');
}

void error_print_json(const error *const e, const string *const s) {
    printf("{\"type\":\"%s\",", error_type_string(e->type));
    switch (e->type) {
        case ERROR_PFX(ERRNO):
            printf("\"errno\":%d", e->data.eno);
            break;
        case ERROR_PFX(PARSER):
            printf("\"stack_trace\":[");
            for (size_t stack_head = 0; stack_head < e->data.parser->stack_head; stack_head++) {
                printf("{\"mode\":\"%s\",\"status\":\"%s\",\"token\":", parser_mode_string(e->data.parser->stack[stack_head].mode), parser_status_string(e->data.parser->stack[stack_head].status));
                token_print_json(e->data.parser->stack[stack_head].t, s);
                putchar('}');
                if (stack_head + 1 < e->data.parser->stack_head) putchar(',');
            }
            printf("]");
            break;
        default:
            printf("null");
            break;
    }
    putchar('}');
}
