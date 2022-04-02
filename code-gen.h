#pragma once
#include <parser.h>

typedef struct {
    ParseTree *tree;
    char *code_string;
    int current; //current place in the code string
}
Generator;

extern char *preamble;
Generator *new_generator(ParseTree *tree);
char *new_code_string();
char *get_str(int val);
void generate_str(Generator *generator, char *string);
void generate_statement(Generator *generator, ASTNode *node);
void generate_declaration_stmt(Generator *generator, ASTNode *node);
void generate_assignment_stmt(Generator *generator, ASTNode *node);
void generate_assignment_dest(Generator *generator, ASTNode *node);
void generate_print_stmt(Generator *generator, ASTNode *node);
void generate_expression(Generator *generator, ASTNode *node);
void generate_for_loop(Generator *generator, ASTNode *node);
void generate_for_clause(Generator *generator, ForLoopClause *for_clause);
void generate_mat_add_sub(Generator *generator, ASTNode *mat1, ASTNode *mat2, char *func_name);
void generate_ord_op(Generator *generator, ASTNode *sca1, ASTNode *sca2, char *chr);
void generate_mat_mul(Generator *generator, ASTNode *mat1, ASTNode *mat2);
void generate_sca_mul(Generator *generator, ASTNode *mat, ASTNode *sca);
void generate_code_string(Generator *generator);