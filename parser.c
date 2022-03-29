#include <string.h>
#include <stdio.h>
#include "defs.h"
#include "tokens.h"
#include "parser.h"
#include "symbol-table.h"

// Internal error means errors that shouldnt be reachable
// due to checks that SHOULD'VE happened before getting to that point.

ASTNode *new_ast_node(ASTNodeType type, int line_number)
{
  ASTNode *node = calloc(1, sizeof(ASTNode));

  node->type = type;
  node->line_number = line_number;

  return node;
}

ParseTree *new_parse_tree()
{
  ParseTree *tree = malloc(sizeof(ParseTree));
  tree->root = new_ast_node(AST_ROOT, 1);
  tree->symbol_table = new_symbol_table();
  return tree;
}

Parser *new_parser(Token *tokenList)
{
  Parser *parser = calloc(1, sizeof(Parser));
  parser->tokens = tokenList;
  parser->token_idx = 0;
  parser->parse_tree = new_parse_tree();
}

Token *get_curr(Parser *parser)
{
  return &parser->tokens[parser->token_idx];
}

int is_eof(Parser *parser)
{
  return (get_curr(parser)->type == TKN_EOF);
}

Token *advance(Parser *parser)
{
  if (is_eof(parser))
  {
    return NULL;
  }
  parser->token_idx++;
  return peek_prev(parser);
}

Token *go_back(Parser *parser)
{
  if (parser->token_idx == 0)
  {
    return NULL;
  }
  parser->token_idx--;
  return peek_next(parser);
}

Token *go_backn(Parser *parser, int n)
{
  if (parser->token_idx < n)
  {
    parser->token_idx = 0;
    return NULL;
  }
  Token *prev = get_curr(parser);
  parser->token_idx -= n;

  return prev;
}

Token *peek_next(Parser *parser)
{
  if (is_eof(parser))
  {
    return NULL;
  }
  return &parser->tokens[parser->token_idx + 1];
}

Token *peek_prev(Parser *parser)
{
  if (parser->token_idx == 0)
  {
    return NULL;
  }
  return &parser->tokens[parser->token_idx - 1];
}

int match(Parser *parser, TokenType type)
{
  if (get_curr(parser)->type == type)
  {
    advance(parser);
    return 1;
  }
  return 0;
}

void advance_to_next_line(Parser *parser)
{
  while (!is_eof(parser) && get_curr(parser)->type != TKN_LINE_FEED)
  {
    advance(parser);
  }
  if (get_curr(parser)->type == TKN_LINE_FEED)
  {
    // Once more to get past the line feed
    advance(parser);
  }
}

void match_or_error(Parser *parser, TokenType type, char *err)
{
  if (!match(parser, type))
  {
    parser_exit_with_error(parser, err);
  }
}

ASTNode *parse_root(Parser *parser, ASTNode *root)
{
  root->contents = calloc(LINE_LIMIT, sizeof(ASTNode));
  root->num_contents = 0;

  while (!is_eof(parser))
  {
    ASTNode *node;
    if (match(parser, TKN_KW_FOR))
    {
      go_back(parser);
      node = parse_for_loop_statement(parser);
    }
    else
    {
      node = parse_statement(parser);
    }
    if (node != NULL)
    {
      add_child(root, *node);
    }
  }

  return root;
}
ASTNode *parse_for_loop_statement(Parser *parser)
{
  ASTNode *node = new_ast_node(AST_FOR_LOOP, get_curr(parser)->line_num);
  node->contents = calloc(LINE_LIMIT, sizeof(ASTNode));
  match_or_error(parser, TKN_KW_FOR, "Expected 'for' keyword in 'for' loop expression (Internal error)");

  match_or_error(parser, TKN_PN_OPENPAREN, "Expected '(' after 'for' keyword in 'for' loop expression.");

  match_or_error(parser, TKN_IDENT, "Expected identifier after '(' in 'for' loop expression.");

  char *ident1 = peek_prev(parser)->contents;
  Variable *var1 = lookup_variable(parser->parse_tree->symbol_table, ident1);
  if (var1 == NULL)
  {
    char error_msg[100];
    snprintf(error_msg, 100, "Variable '%s' not declared in 'for' loop expression.", ident1);
    parser_exit_with_error(parser, error_msg);
  }

  if (match(parser, TKN_PN_COMMA))
  {
    // Doubly nested for loop
    match_or_error(parser, TKN_IDENT, "Expected identifier after ',' in 'for' loop expression.");
    char *ident2 = peek_prev(parser)->contents;
    Variable *var2 = lookup_variable(parser->parse_tree->symbol_table, ident1);
    if (var2 == NULL)
    {
      char error_msg[100];
      snprintf(error_msg, 100, "Variable '%s' not declared in 'for' loop expression.", ident1);
      parser_exit_with_error(parser, error_msg);
    }

    match_or_error(parser, TKN_KW_IN, "Expected 'in' keyword after second identifier in 'for' loop expression.");

    ASTNode *expr1 = parse_expression(parser);
    match_or_error(parser, TKN_PN_COLON, "Expected ':' after first expression in 'for' loop expression.");
    ASTNode *expr2 = parse_expression(parser);
    match_or_error(parser, TKN_PN_COLON, "Expected ':' after second expression in 'for' loop expression.");
    ASTNode *expr3 = parse_expression(parser);
    ForLoopClause *clause = calloc(1, sizeof(ForLoopClause));
    clause->var = var1;
    clause->expr1 = expr1;
    clause->expr2 = expr2;
    clause->expr3 = expr3;

    match_or_error(parser, TKN_PN_COMMA, "Expected ',' after third expression in 'for' loop expression.");
    ASTNode *expr4 = parse_expression(parser);
    match_or_error(parser, TKN_PN_COLON, "Expected ':' after fourth expression in 'for' loop expression.");
    ASTNode *expr5 = parse_expression(parser);
    match_or_error(parser, TKN_PN_COLON, "Expected ':' after fifth expression in 'for' loop expression.");
    ASTNode *expr6 = parse_expression(parser);
    ForLoopClause *clause2 = calloc(1, sizeof(ForLoopClause));
    clause2->var = var2;
    // TODO: should we type-check here?
    clause2->expr1 = expr4;
    clause2->expr2 = expr5;
    clause2->expr3 = expr6;

    match_or_error(parser, TKN_PN_CLOSEPAREN, "Expected ')' after sixth expression in 'for' loop expression.");
    match_or_error(parser, TKN_PN_OPENBRACE, "Expected '{' after for clause.");

    node->for_clause_1 = clause;
    node->for_clause_2 = clause2;
  }
  else if (match(parser, TKN_KW_IN))
  {
    // Single nested for loop
    ASTNode *expr1 = parse_expression(parser);
    match_or_error(parser, TKN_PN_COLON, "Expected ':' after first expression in 'for' loop expression.");
    ASTNode *expr2 = parse_expression(parser);
    match_or_error(parser, TKN_PN_COLON, "Expected ':' after second expression in 'for' loop expression.");
    ASTNode *expr3 = parse_expression(parser);
    ForLoopClause *clause = calloc(1, sizeof(ForLoopClause));
    clause->var = var1;
    // TODO: should we type-check here?
    clause->expr1 = expr1;
    clause->expr2 = expr2;
    clause->expr3 = expr3;
    match_or_error(parser, TKN_PN_CLOSEPAREN, "Expected ')' after last expression in 'for' loop expression.");
    match_or_error(parser, TKN_PN_OPENBRACE, "Expected '{' after for clause.");
    node->for_clause_1 = clause;
  }
  else
  {
    parser_exit_with_error(parser, "Expected ':' or 'in' after identifier in 'for' loop expression.");
  }
  match_or_error(parser, TKN_LINE_FEED, "Expected '\n' after for statement.");
  while (get_curr(parser)->type != TKN_PN_CLOSEBRACE && !is_eof(parser))
  {
    ASTNode *statement = parse_statement(parser);
    if (statement != NULL)
    {
      add_child(node, *statement);
    }
  }
  match_or_error(parser, TKN_PN_CLOSEBRACE, "Expected '}' after for statement.");
  return node;
}

ASTNode *parse_statement(Parser *parser)
{
  if (match(parser, TKN_IDENT))
  {
    go_back(parser);
    return parse_assignment(parser);
  }
  else if (match(parser, TKN_TYPE_MATRIX) || match(parser, TKN_TYPE_SCALAR) || match(parser, TKN_TYPE_VECTOR))
  {
    go_back(parser);
    return parse_declaration(parser);
  }
  else if (match(parser, TKN_SPECIAL_PRINT) || match(parser, TKN_SPECIAL_PRINTSEP))
  {
    go_back(parser);
    return parse_print_statement(parser);
  }
}

ASTNode *parse_print_statement(Parser *parser)
{

  ASTNode *node = new_ast_node(AST_STMT, get_curr(parser)->line_num);
  node->stmt_type = STMT_PRINT_STMT;
  // TODO: Continue
  if (match(parser, TKN_SPECIAL_PRINT))
  {
    match_or_error(parser, TKN_PN_OPENPAREN, "Expected '(' after print keyword");
    ASTNode *expr = parse_expression(parser);
    node->contents = expr;
    node->num_contents = 1;
    match_or_error(parser, TKN_PN_CLOSEPAREN, "Expected ')' after print inner expression");
  }
  else
  {
    match_or_error(parser, TKN_PN_OPENPAREN, "Expected '(' after printsep keyword");
    match_or_error(parser, TKN_PN_CLOSEPAREN, "Expected ')' after printsep start");
  }
  match_or_error(parser, TKN_LINE_FEED, "Expected statement to end with a newline");

  return node;
}

ASTNode *parse_assignment(Parser *parser)
{
  ASTNode *node = new_ast_node(AST_STMT, get_curr(parser)->line_num);

  node->stmt_type = STMT_ASSIGNMENT;
  ASTNode *lhs = parse_assignment_dest(parser);
  match_or_error(parser, TKN_ASSIGN, "Expected '=' after identifier");
  node->lhs = lhs;

  // Optionally parse list expression
  if (match(parser, TKN_PN_OPENBRACE))
  {
    go_back(parser);
    ASTNode *list_expr = parse_list_expression(parser, lhs->var_type);
    node->rhs = list_expr;
  }
  else
  {
    ASTNode *rhs = parse_expression(parser);
    node->rhs = rhs;
  }

  match_or_error(parser, TKN_LINE_FEED, "Expected statement to end with a newline");
  return node;
}

ASTNode *parse_declaration(Parser *parser)
{
  ASTNode *node = new_ast_node(AST_STMT, get_curr(parser)->line_num);
  node->stmt_type = STMT_DECL;

  // All the verification work is done in this function:
  ResultType type = parse_var_type(parser);

  match_or_error(parser, TKN_IDENT, "Expected identifier after type name");
  char *ident = peek_prev(parser)->contents;

  node->var_name = ident;
  node->var_type = type;

  Variable var = {.name = ident, .type = type};

  Variable *lookup = lookup_variable(parser->parse_tree->symbol_table, ident);
  if (lookup != NULL)
  {
    parser_exit_with_error(parser, "Redeclaration of variable");
  }
  add_variable(parser->parse_tree->symbol_table, var);

  advance_to_next_line(parser);
  return node;
}

// Returns where it started
// Validates the declaration as well as parsing the type
ResultType parse_var_type(Parser *parser)
{
  // Guaranteed to not be undefined since we checked that the token is a type
  // keyword before.
  VariableType var_type = get_var_type(get_curr(parser)->type);

  int go_back_count = 0;
  ResultType type = {.var_type = var_type, .height = 1, .width = 1};
  match_or_error(parser, TKN_IDENT, "Expected identifier after type name");
  go_back_count++;

  if (type.var_type == TYPE_VECTOR)
  {
    match_or_error(parser, TKN_PN_OPENBRACKET, "Expected '[' after vector declaration");

    match_or_error(parser, TKN_INT_LITERAL, "Expected integer after '['");

    type.height = atoi(peek_prev(parser)->contents);

    match_or_error(parser, TKN_PN_CLOSEBRACKET, "Expected ']' after vector size");
    go_back_count += 3;
  }
  else if (type.var_type == TYPE_MATRIX)
  {
    match_or_error(parser, TKN_PN_OPENBRACKET, "Expected '[' after matrix identifier");

    match_or_error(parser, TKN_INT_LITERAL, "Expected integer after '['");

    type.height = atoi(peek_prev(parser)->contents);

    match_or_error(parser, TKN_PN_COMMA, "Expected ',' after matrix width");
    match_or_error(parser, TKN_INT_LITERAL, "Expected integer after ','");

    type.width = atoi(peek_prev(parser)->contents);

    match_or_error(parser, TKN_PN_CLOSEBRACKET, "Expected ']' after matrix size");
    go_back_count += 5;
  }
  else if (type.var_type == TYPE_SCALAR)
  {
    match_or_error(parser, TKN_IDENT, "Expected an identifier after scalar keyword.");
    go_back_count++;
  }

  match_or_error(parser, TKN_LINE_FEED, "Expected declaration to end with a newline");
  go_back_count++;

  go_backn(parser, go_back_count);
  return type;
}

// Does NOT parse list expressions
// List expressions are only parsed on the immediate RHS of an assignment
ASTNode *parse_expression(Parser *parser)
{
  return parse_term(parser);
}

ASTNode *parse_term(Parser *parser)
{
  ASTNode *expr = parse_factor(parser);

  while (match(parser, TKN_PLUS) || match(parser, TKN_MINUS))
  {
    Token *op = peek_prev(parser);
    OperatorType type = get_op_type(op->type);
    ASTNode *rhs = parse_factor(parser);
    ASTNode *parent = new_ast_node(AST_EXPR, get_curr(parser)->line_num);

    parent->exp_type = EXP_BINOP;
    parent->op_type = type;
    parent->lhs = expr;
    parent->rhs = rhs;
    parent->exp_result_type = get_operation_result_type(parser, type, parent->lhs->exp_result_type, parent->rhs->exp_result_type);
    expr = parent;
  }

  return expr;
}

ASTNode *parse_factor(Parser *parser)
{
  ASTNode *expr = parse_atomic(parser);

  while (match(parser, TKN_PLUS) || match(parser, TKN_MINUS))
  {
    Token *op = peek_prev(parser);
    OperatorType type = get_op_type(op->type);
    ASTNode *rhs = parse_factor(parser);
    ASTNode *parent = new_ast_node(AST_EXPR, get_curr(parser)->line_num);

    parent->exp_type = EXP_BINOP;
    parent->op_type = type;
    parent->lhs = expr;
    parent->rhs = rhs;
    parent->exp_result_type = get_operation_result_type(parser, type, parent->lhs->exp_result_type, parent->rhs->exp_result_type);
    expr = parent;
  }

  return expr;
}

ASTNode *parse_atomic(Parser *parser)
{
  if (match(parser, TKN_PN_OPENPAREN))
  {
    ASTNode *expr = parse_expression(parser);
    match_or_error(parser, TKN_PN_CLOSEPAREN, "Expected ')' after expression for parenthesized expression");
    return expr;
  }
  if (match(parser, TKN_IDENT))
  {
    Variable *var = lookup_variable(parser->parse_tree->symbol_table, peek_prev(parser)->contents);

    // Either identifier, or index expression
    if (match(parser, TKN_PN_OPENBRACKET))
    {
      // Parse Index expr
      if (var->type.var_type == TYPE_SCALAR)
      {
        parser_exit_with_error(parser, "Cannot index into a scalar variable");
      }
      ASTNode *expr1 = parse_expression(parser);
      ASTNode *index_expr = new_ast_node(AST_EXPR, get_curr(parser)->line_num);
      index_expr->exp_type = EXP_INDEX;
      // Index always returns scalar
      index_expr->exp_result_type = (ResultType){.var_type = TYPE_SCALAR, .height = 1, .width = 1};
      if (var->type.var_type == TYPE_VECTOR)
      {
        match_or_error(parser, TKN_PN_CLOSEBRACKET, "Expected ']' after expression for index expression into vector.");

        index_expr->contents = expr1;
        index_expr->num_contents = 1;

        return index_expr;
      }
      // Type is Matrix
      match_or_error(parser, TKN_PN_COMMA, "Expected a comma after the first expression in a matrix index expression");

      ASTNode *expr2 = parse_expression(parser);
      match_or_error(parser, TKN_PN_CLOSEBRACKET, "Expected ']' after expression for index expression into matrix.");
      index_expr->contents = calloc(2, sizeof(ASTNode));
      index_expr->contents[0] = *expr1;
      index_expr->contents[1] = *expr2;
      // free(expr1);
      // free(expr2);
      index_expr->num_contents = 2;

      return index_expr;
    }
    else
    {
      ASTNode *identifier_exp = new_ast_node(AST_EXPR, get_curr(parser)->line_num);
      identifier_exp->exp_type = EXP_IDENT;
      identifier_exp->ident = peek_prev(parser)->contents;
      identifier_exp->exp_result_type = var->type;
      return identifier_exp;
    }
  }
  if (match(parser, TKN_INT_LITERAL) || match(parser, TKN_FLOAT_LITERAL))
  {
    ASTNode *literal_expr = new_ast_node(AST_EXPR, peek_prev(parser)->line_num);
    literal_expr->exp_type = EXP_LITERAL;
    literal_expr->exp_result_type = (ResultType){.var_type = TYPE_SCALAR, .height = 1, .width = 1};
    if (peek_prev(parser)->type == TKN_INT_LITERAL)
    {
      literal_expr->literal_value = (double)atoi(peek_prev(parser)->contents);
    }
    else
    {
      literal_expr->literal_value = atof(peek_prev(parser)->contents);
    }
    return literal_expr;
  }
  if (match(parser, TKN_FUNCTION_CHOOSE) || match(parser, TKN_FUNCTION_SQRT) || match(parser, TKN_FUNCTION_CHOOSE))
  {
    TokenType func_tok = peek_prev(parser)->type;
    int arg_count = get_arg_count(func_tok);
    match_or_error(parser, TKN_PN_OPENPAREN, "Expected '(' after function call");
    int idx = 0;
    ASTNode *contents = calloc(arg_count, sizeof(ASTNode));

    while (idx < arg_count)
    {
      if (get_curr(parser)->type == TKN_PN_CLOSEPAREN)
      {
        // Provide better error message in this case.
        parser_exit_with_error(parser, "Argument list ended before expected number of arguments");
      }
      contents[idx] = *parse_expression(parser);
      idx++;
      if (idx < arg_count)
      {
        match_or_error(parser, TKN_PN_COMMA, "Expected a comma after expression inside function call.");
      }
    }
    match_or_error(parser, TKN_PN_CLOSEPAREN, "Expected ')' after function call.");
    ASTNode *func_call = new_ast_node(AST_EXPR, peek_prev(parser)->line_num);
    func_call->exp_type = EXP_FUNC_CALL;
    func_call->exp_result_type = get_func_call_result_type(parser, func_tok, contents);
    func_call->num_contents = arg_count;
    return func_call;
  }
  parser_exit_with_error(parser, "Unexpected token when looking for a factor");
}

ResultType get_func_call_result_type(Parser *parser, TokenType func_tok, ASTNode *contents)
{
  ResultType result_type = {.var_type = TYPE_SCALAR, .height = 1, .width = 1};
  if (func_tok == TKN_FUNCTION_CHOOSE)
  {
    for (int i = 0; i < 4; i++)
    {
      if (contents[i].exp_result_type.var_type != TYPE_SCALAR)
      {
        parser_exit_with_error(parser, "Expected scalar expression in choose function call.");
      }
    }
    return result_type;
  }
  else if (func_tok == TKN_FUNCTION_SQRT)
  {
    if (contents[0].exp_result_type.var_type != TYPE_SCALAR)
    {
      parser_exit_with_error(parser, "Expected scalar expression in sqrt function call.");
    }
    return result_type;
  }
  else if (func_tok == TKN_FUNCTION_TR)
  {
    if (contents[0].exp_result_type.var_type == TYPE_SCALAR)
    {
      parser_exit_with_error(parser, "Expected matrix or vector expression in tr function call.");
    }
    result_type.var_type = TYPE_MATRIX;
    result_type.height = contents[0].exp_result_type.width;
    result_type.width = contents[0].exp_result_type.height;
  }
  return result_type;
}

ResultType get_operation_result_type(Parser *parser, OperatorType op_type, ResultType lhs, ResultType rhs)
{
  ResultType result_type = {.var_type = TYPE_SCALAR, .height = 1, .width = 1};
  if (op_type == OP_PLUS || op_type == OP_MINUS)
  {
    if (lhs.var_type == TYPE_SCALAR && rhs.var_type == TYPE_SCALAR)
    {
      result_type.var_type = TYPE_SCALAR;
    }
    else if (lhs.var_type == TYPE_VECTOR && rhs.var_type == TYPE_VECTOR)
    {
      if (lhs.height != rhs.height)
      {
        parser_exit_with_error(parser, "Expected vectors of equal length in vector addition/subtraction.");
      }
      result_type.var_type = TYPE_VECTOR;
      result_type.height = lhs.height;
      result_type.width = 1;
    }
    else if (lhs.var_type == TYPE_MATRIX && rhs.var_type == TYPE_MATRIX)
    {
      if (lhs.height != rhs.height || lhs.width != rhs.width)
      {
        parser_exit_with_error(parser, "Expected matrices of equal size in matrix addition/subtraction.");
      }
      result_type.var_type = TYPE_MATRIX;
      result_type.height = lhs.height;
      result_type.width = lhs.width;
    }
    else
    {
      parser_exit_with_error(parser, "Expected types to match in addition/subtraction.");
    }
  }
  else if (op_type == OP_MULT)
  {
    if (lhs.var_type == TYPE_SCALAR && rhs.var_type == TYPE_SCALAR)
    {
      result_type.var_type = TYPE_SCALAR;
    }
    else if (lhs.var_type == TYPE_SCALAR && rhs.var_type != TYPE_SCALAR)
    {
      result_type = rhs;
    }
    else if (rhs.var_type == TYPE_SCALAR && lhs.var_type != TYPE_SCALAR)
    {
      result_type = lhs;
    }
    else
    {
      // Both are matrix/vector types
      result_type.var_type = TYPE_MATRIX;
      result_type.height = lhs.height;
      result_type.width = rhs.width;
      if (lhs.width != rhs.height)
      {
        parser_exit_with_error(parser, "Expected matrices of compatible sizes in matrix multiplication.");
      }
    }
  }
  return result_type;
}

ASTNode *parse_assignment_dest(Parser *parser)
{
  ASTNode *dest = new_ast_node(AST_ASSIGNMENT_DEST, get_curr(parser)->line_num);
  // Should not print error
  match_or_error(parser, TKN_IDENT, "Expected an identifier for assignment (Internal error).");
  dest->var_name = peek_prev(parser)->contents;
  Variable *var = lookup_variable(parser->parse_tree->symbol_table, dest->var_name);
  if (var == NULL)
  {
    char error_msg[100];
    snprintf(error_msg, 100, "Variable '%s' not declared.", dest->var_name);
    parser_exit_with_error(parser, error_msg);
  }
  dest->var_type = var->type;
  dest->is_indexed_assignment = 0;

  if (match(parser, TKN_PN_OPENBRACKET))
  {
    // Assigning to an index
    dest->is_indexed_assignment = 1;

    if (var->type.var_type == TYPE_SCALAR)
      parser_exit_with_error(parser, "Expected a vector or matrix for indexing but got a scalar.");

    match_or_error(parser, TKN_INT_LITERAL, "Expected an integer when indexing on the left side of an assignment.");
    dest->index_1 = atoi(peek_prev(parser)->contents);

    if (var->type.var_type == TYPE_VECTOR)
    {
      dest->index_2 = 0;
      match_or_error(parser, TKN_PN_CLOSEBRACKET, "Expected a closing bracket after the first index for a vector.");
      return dest;
    }
    match_or_error(parser, TKN_PN_COMMA, "Expected a comma after an integer when indexing a matrix on the left side of an assignment.");

    match_or_error(parser, TKN_INT_LITERAL, "Expected a second index when indexing a matrix on the left side of an assignment.");

    dest->index_2 = atoi(peek_prev(parser)->contents);
  }
  return dest;
}
ASTNode *parse_list_expression(Parser *parser, ResultType type)
{
  ASTNode *list_node = new_ast_node(AST_EXPR, get_curr(parser)->line_num);
  list_node->exp_type = EXP_LIST;
  list_node->exp_result_type = type;

  ASTNode *contents = calloc(256, sizeof(ASTNode));
  match_or_error(parser, TKN_PN_OPENBRACE, "Expected '{' for list expression");
  int idx = 0;
  while (match(parser, TKN_INT_LITERAL) || match(parser, TKN_FLOAT_LITERAL))
  {
    ASTNode *literal_node = new_ast_node(AST_EXPR, get_curr(parser)->line_num);
    literal_node->exp_type = EXP_LITERAL;
    literal_node->exp_result_type = (ResultType){.var_type = TYPE_SCALAR, .height = 1, .width = 1};
    if (peek_prev(parser)->type == TKN_INT_LITERAL)
    {
      literal_node->literal_value = atoi(peek_prev(parser)->contents);
    }
    else
    {
      literal_node->literal_value = atof(peek_prev(parser)->contents);
    }
    contents[idx] = *literal_node;
    idx++;
    // free(literal_node);
  }
  list_node->num_contents = idx;
  if (list_node->num_contents != type.height * type.width)
  {

    char error_str[100];
    snprintf(error_str, 100, "Expected %d elements in list expression, got %d.", type.height * type.width, list_node->num_contents);

    parser_exit_with_error(parser, error_str);
  }

  match_or_error(parser, TKN_PN_CLOSEBRACE, "Expected '}' after last number literal inside list expression");
  return list_node;
}

void add_child(ASTNode *parent, ASTNode child)
{
  parent->contents[parent->num_contents] = child;
  parent->num_contents++;
}

int get_arg_count(TokenType type)
{
  switch (type)
  {
  case TKN_FUNCTION_CHOOSE:
    return 4;
  case TKN_FUNCTION_SQRT:
    return 1;
  case TKN_FUNCTION_TR:
    return 1;
  default:
    return 0;
  }
}
OperatorType get_op_type(TokenType type)
{
  switch (type)
  {
  case TKN_MINUS:
    return OP_MINUS;
  case TKN_PLUS:
    return OP_PLUS;
  case TKN_MULT:
    return OP_MULT;
  }
}
void parser_exit_with_error(Parser *parser, char *message)
{
  printf("Error: (Line %d) %s\n", message, get_curr(parser)->line_num);
  exit(1);
}