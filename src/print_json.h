
#pragma once

#include <stdlib.h>
#include <stdio.h>
#include "parser.h"
#include "error.h"
#include "infer.h"

void token_print_json(const token *const t, const string *const s);

void symbol_table_bucket_print_json(const symbol_table_bucket *const b);

void symbol_table_print_json(const symbol_table *const table);

void var_type_print_json(const var_type *const t);

void ast_node_link_print_json(ast_node_link *head, const string *const s);

void ast_vec_node_print_json(const ast_vec_node *const vec, const string *const s);

void ast_fn_node_print_json(const ast_fn_node *const fn, const string *const s);

void ast_call_node_print_json(const ast_call_node *const c, const string *const s);

void ast_if_node_print_json(const ast_if_node *const if_node, const string *const s);

void ast_node_print_json(const ast_node *const node, const string *const s);

void error_print_json(const error *const e, const string *const s);
