#pragma once
#include "tokens.h"
#include "symbol-table.h"

typedef enum ASTNodeType
{
  AST_ROOT,
  AST_STMT,
  AST_FOR_LOOP,
  AST_EXPR,
  AST_ASSIGNMENT_DEST
} ASTNodeType;

typedef enum StatementType
{
  // var_type -> VariableType, var_name -> Name
  // Only allowed in the global scope
  STMT_DECL,
  // lhs -> AST_ASSIGNMENT_DEST, rhs->AST_EXPR
  STMT_ASSIGNMENT,
  STMT_PRINT_STMT
} StatementType;

typedef enum ExpressionType
{
  EXP_LITERAL,
  EXP_IDENT,
  EXP_BINOP,
  EXP_FUNC_CALL,
  EXP_INDEX,
  // List expressions are only allowed directly on the RHS of an assignment
  EXP_LIST
} ExpressionType;

typedef enum OperatorType
{
  OP_PLUS,
  OP_MINUS,
  OP_MULT,
} OperatorType;

typedef struct ForLoopClause
{
  Variable *var;

  ASTNode *expr1;
  ASTNode *expr2;
  ASTNode *expr3;
} ForLoopClause;

typedef struct ASTNode
{
  ASTNodeType type;

  StatementType stmt_type;

  // AST_ASSIGNMENT_DEST
  int is_indexed_assignment; // 0 if not indexed, 1 if indexed
  int index_1;
  int index_2; // Will always be 0 for vector destinations

  // AST_DECL || AST_ASSIGNMENT_DEST
  char *var_name;
  ResultType var_type;

  // AST_EXPR
  ExpressionType exp_type;

  // EXP_LITERAL
  double literal_value;

  // EXP_IDENT
  char *ident;

  // EXP_BINOP
  OperatorType op_type;

  // EXP_BINOP || STMT_ASSIGNMENT
  ASTNode *lhs;
  ASTNode *rhs;

  // AST_EXPR
  ResultType exp_result_type;

  // If a node has multiple ASTNodes nested inside,
  // this is where they are stored
  // AST_FOR_LOOP || AST_ROOT || EXP_INDEX || EXP_FUNC_CALL
  ASTNode *contents;
  int num_contents;

  // AST_FOR_LOOP
  ForLoopClause *for_clause_1;
  // CAN BE NULL! (Is not null if it's a double for loop)
  ForLoopClause *for_clause_2;

  int line_number;
} ASTNode;

typedef struct ParseTree
{
  ASTNode *root;
  SymbolTable *symbol_table;
} ParseTree;

typedef struct Parser
{
  Token *tokens;
  int token_idx;
  ParseTree *parse_tree;
} Parser;
ASTNode *new_ast_node(ASTNodeType type, int line_number);
ParseTree *new_parse_tree();
Parser *new_parser(Token *tokenList);
Token *get_curr(Parser *parser);
int is_eof(Parser *parser);
Token *advance(Parser *parser);
Token *go_back(Parser *parser);
Token *go_backn(Parser *parser, int n);
Token *peek_next(Parser *parser);
Token *peek_prev(Parser *parser);
int match(Parser *parser, TokenType type);
void advance_to_next_line(Parser *parser);
void match_or_error(Parser *parser, TokenType type, char *err);
ASTNode *parse_root(Parser *parser, ASTNode *root);
ASTNode *parse_for_loop_statement(Parser *parser);
ASTNode *parse_statement(Parser *parser);
ASTNode *parse_print_statement(Parser *parser);
ASTNode *parse_assignment(Parser *parser);
ASTNode *parse_declaration(Parser *parser);
ResultType parse_var_type(Parser *parser);
ASTNode *parse_expression(Parser *parser);
ASTNode *parse_factor(Parser *parser);
ResultType get_func_call_result_type(Parser *parser, TokenType func_tok, ASTNode *contents);
ResultType get_operation_result_type(Parser *parser, OperatorType op_type, ResultType lhs, ResultType rhs);
ASTNode *parse_assignment_dest(Parser *parser);
ASTNode *parse_list_expression(Parser *parser, ResultType type);
void add_child(ASTNode *parent, ASTNode child);
int get_arg_count(TokenType type);
OperatorType get_op_type(TokenType type);
void parser_exit_with_error(Parser *parser, char *message);