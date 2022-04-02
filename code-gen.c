#include <stdio.h>
#include <string.h>
#include "code-gen.h"
#include "defs.h"

char *preamble = "#include \"preamble.h\"\n";

Generator *new_generator(ParseTree *tree)
{
    Generator *generator = calloc(1, sizeof(Generator));
    generator->tree = tree;
    generator->code_string = calloc(1, 10 * LINE_LIMIT * LINE_LIMIT); // not sure
    return generator;
}

void generate_new_code_string(Generator *generator)
{
    gen(generator, preamble);
    gen(generator, "#include <stdio.h>\n\nint main()\n{\n");
}

void generate_identifier(Generator *generator, char *str)
{
    gen(generator, "_");
    gen(generator, str);
}

char *get_str(int val)
{
    char *str = malloc(LINE_LIMIT * sizeof(char));
    sprintf(str, "%d", val);
    return str;
}

void gen(Generator *generator, char *str)
{
    strcat(generator->code_string, str);
}

void generate_statement(Generator *generator, ASTNode *node)
{
    switch (node->stmt_type)
    {
    case STMT_DECL:
        generate_declaration_stmt(generator, node);
        break;
    case STMT_ASSIGNMENT:
        generate_assignment_stmt(generator, node);
        break;
    case STMT_PRINT_STMT:
        generate_print_stmt(generator, node);
        break;
    }
}

void generate_declaration_stmt(Generator *generator, ASTNode *node)
{
    if (node->var_type.var_type == TYPE_SCALAR)
    {
        gen(generator, "double ");
        generate_identifier(generator, node->var_name);
        gen(generator, "= 0;");
    }
    else if (node->var_type.var_type == TYPE_VECTOR)
    {
        gen(generator, "double **");
        generate_identifier(generator, node->var_name);
        gen(generator, " = allocate_matrix(");
        gen(generator, get_str(node->var_type.height));
        gen(generator, ", ");
        gen(generator, get_str(node->var_type.width));
        gen(generator, ");");
    }
    else if (node->var_type.var_type == TYPE_MATRIX)
    {
        gen(generator, "double **");
        generate_identifier(generator, node->var_name);
        gen(generator, " = allocate_matrix(");
        gen(generator, get_str(node->var_type.height));
        gen(generator, ", ");
        gen(generator, get_str(node->var_type.width));
        gen(generator, ");");
    }
}

void generate_assignment_stmt(Generator *generator, ASTNode *node)
{
    if (node->rhs->exp_type == EXP_LIST)
    {
        ASTNode *list = node->rhs;
        gen(generator, "assign_to_mat(");
        generate_identifier(generator, node->lhs->ident);
        gen(generator, ", ");
        gen(generator, get_str(list->exp_result_type.height));
        gen(generator, ", ");
        gen(generator, get_str(list->exp_result_type.width));
        gen(generator, ", ");
        for (int i = 0; i < list->num_contents - 1; i++)
        {
            gen(generator, "(double)");
            gen(generator, list->contents[i].literal_str);
            gen(generator, ", ");
        }
        gen(generator, "(double)");
        gen(generator, list->contents[list->num_contents - 1].literal_str);
        gen(generator, ")");
    }
    else
    {
        if (node->lhs->exp_type == EXP_IDENT && node->lhs->exp_result_type.var_type != TYPE_SCALAR)
        {
            gen(generator, "assign_mat_to_mat(");
            generate_identifier(generator, node->lhs->ident);
            gen(generator, ", ");
            generate_expression(generator, node->rhs);
            gen(generator, ", ");
            gen(generator, get_str(node->lhs->exp_result_type.height));
            gen(generator, ", ");
            gen(generator, get_str(node->lhs->exp_result_type.width));
            gen(generator, ")");
        }
        else
        {
            generate_assignment_dest(generator, node->lhs);
            gen(generator, " = ");
            generate_expression(generator, node->rhs);
        }
    }
    gen(generator, ";");
}

void generate_assignment_dest(Generator *generator, ASTNode *node)
{
    generate_identifier(generator, node->ident);
    if (node->exp_type == EXP_INDEX)
    {
        gen(generator, "[get_int(");
        generate_expression(generator, node->contents);
        gen(generator, ")]");
        if (node->num_contents == 2)
        {
            gen(generator, "[gen_int(");
            generate_expression(generator, &node->contents[1]);
            gen(generator, ")]");
        }
    }
}

void generate_print_stmt(Generator *generator, ASTNode *node)
{
    if (node->num_contents == 0)
    {
        gen(generator, "printsep();");
        return;
    }
    ResultType type = node->contents[0].exp_result_type;
    switch (type.var_type)
    {
    case TYPE_SCALAR:
        gen(generator, "print(");
        break;
    case TYPE_VECTOR:
    case TYPE_MATRIX:
        gen(generator, "print_mat(");
        break;
    default:
        printf("Uncaught error with the parser.");
        exit(1);
        break;
    }
    generate_expression(generator, node->contents);
    if (type.var_type == TYPE_VECTOR || type.var_type == TYPE_MATRIX)
    {
        gen(generator, ", ");
        gen(generator, get_str(type.height));
        gen(generator, ", ");
        gen(generator, get_str(type.width));
    }
    gen(generator, ");");
}

void generate_expression(Generator *generator, ASTNode *node)
{
    switch (node->exp_type)
    {
    case EXP_LITERAL:
        gen(generator, node->literal_str);
        break;
    case EXP_LIST:
        gen(generator, "{");
        for (int i = 0; i < node->num_contents - 1; i++)
        {
            gen(generator, (&node->contents[i])->literal_str);
            gen(generator, ",");
        }
        gen(generator, (&node->contents[node->num_contents - 1])->literal_str);
        gen(generator, "}");
        break;
    case EXP_IDENT:
        generate_identifier(generator, node->ident);
        break;
    case EXP_BINOP:
        gen(generator, "(");
        if (node->op_type == OP_PLUS)
        {
            if (node->lhs->exp_result_type.var_type == TYPE_MATRIX || node->lhs->exp_result_type.var_type == TYPE_VECTOR)
            {
                generate_mat_add_sub(generator, node->lhs, node->rhs, "mat_add");
            }
            else if (node->lhs->exp_result_type.var_type == TYPE_SCALAR)
            {
                generate_ord_op(generator, node->lhs, node->rhs, "+");
            }
        }
        else if (node->op_type == OP_MINUS)
        {
            if (node->lhs->exp_result_type.var_type == TYPE_MATRIX || node->lhs->exp_result_type.var_type == TYPE_VECTOR)
            {
                generate_mat_add_sub(generator, node->lhs, node->rhs, "mat_sub");
            }
            else if (node->lhs->exp_result_type.var_type == TYPE_SCALAR)
            {
                generate_ord_op(generator, node->lhs, node->rhs, "-");
            }
        }
        else if (node->op_type == OP_MULT)
        {
            if (node->lhs->exp_result_type.var_type == TYPE_MATRIX || node->lhs->exp_result_type.var_type == TYPE_VECTOR)
            {
                if (node->rhs->exp_result_type.var_type == TYPE_MATRIX || node->rhs->exp_result_type.var_type == TYPE_VECTOR)
                {
                    generate_mat_mul(generator, node->lhs, node->rhs);
                }
                else if (node->rhs->exp_result_type.var_type == TYPE_SCALAR)
                {
                    generate_sca_mul(generator, node->lhs, node->rhs);
                }
            }
            else if (node->lhs->exp_result_type.var_type == TYPE_SCALAR)
            {
                if (node->rhs->exp_result_type.var_type == TYPE_MATRIX || node->rhs->exp_result_type.var_type == TYPE_VECTOR)
                {
                    generate_mat_mul(generator, node->rhs, node->lhs);
                }
                else if (node->rhs->exp_result_type.var_type == TYPE_SCALAR)
                {
                    generate_ord_op(generator, node->lhs, node->rhs, "*");
                }
            }
        }
        gen(generator, ")");
        break;
    case EXP_FUNC_CALL:
        generate_identifier(generator, node->ident);
        gen(generator, "(");
        for (int i = 0; i < node->num_contents - 1; i++)
        {
            generate_expression(generator, &node->contents[i]);
            gen(generator, ",");
        }
        generate_expression(generator, &node->contents[node->num_contents - 1]);
        if (strcmp(node->ident, "tr") == 0)
        {
            gen(generator, ", ");
            gen(generator, get_str(node->contents->exp_result_type.height));
            gen(generator, ", ");
            gen(generator, get_str(node->contents->exp_result_type.width));
        }
        gen(generator, ")");
        break;
    case EXP_INDEX:
        generate_identifier(generator, node->ident);
        if (node->num_contents == 1)
        {
            gen(generator, "[get_int(");
            generate_expression(generator, node->contents);
            gen(generator, ") - 1][0]");
        }
        else if (node->num_contents == 2)
        {
            gen(generator, "[get_int(");
            generate_expression(generator, node->contents);
            gen(generator, ") - 1][get_int(");
            generate_expression(generator, &node->contents[1]);
            gen(generator, ") - 1]");
        }
        break;
    }
}

void generate_for_loop(Generator *generator, ASTNode *node)
{
    gen(generator, "\t");
    generate_for_clause(generator, node->for_clause_1);
    gen(generator, "\n\t{\n");
    if (node->for_clause_2 != NULL)
    {
        gen(generator, "\t\t");
        generate_for_clause(generator, node->for_clause_2);
        gen(generator, "\n{\n");
    }
    for (int i = 0; i < node->num_contents; i++)
    {
        gen(generator, "\t\t");
        if (node->for_clause_2 != NULL)
        {
            gen(generator, "\t");
        }
        generate_statement(generator, &node->contents[i]);
        gen(generator, "\n");
    }
    if (node->for_clause_2 != NULL)
    {
        gen(generator, "\t\t}\n");
    }
    gen(generator, "\t}\n");
}

void generate_for_clause(Generator *generator, ForLoopClause *for_clause)
{
    char *var_name = for_clause->var->name;
    gen(generator, "for (");
    generate_identifier(generator, var_name);
    gen(generator, " = ");
    generate_expression(generator, for_clause->expr1);
    gen(generator, "; ");
    gen(generator, "get_int(");
    generate_identifier(generator, var_name);
    gen(generator, ") <= get_int(");
    generate_expression(generator, for_clause->expr2);
    gen(generator, "); ");
    generate_identifier(generator, var_name);
    gen(generator, " += ");
    generate_expression(generator, for_clause->expr3);
    gen(generator, ")");
}

void generate_mat_add_sub(Generator *generator, ASTNode *mat1, ASTNode *mat2, char *func_name)
{
    gen(generator, func_name);
    gen(generator, "(");
    generate_expression(generator, mat1);
    gen(generator, ", ");
    generate_expression(generator, mat2);
    gen(generator, ", ");
    gen(generator, get_str(mat1->exp_result_type.height));
    gen(generator, ", ");
    gen(generator, get_str(mat1->exp_result_type.width));
    gen(generator, ")");
}

void generate_ord_op(Generator *generator, ASTNode *sca1, ASTNode *sca2, char *chr)
{
    generate_expression(generator, sca1);
    gen(generator, chr);
    generate_expression(generator, sca2);
}

void generate_mat_mul(Generator *generator, ASTNode *mat1, ASTNode *mat2)
{
    gen(generator, "mat_mul(");
    generate_expression(generator, mat1);
    gen(generator, ", ");
    gen(generator, get_str(mat1->exp_result_type.height));
    gen(generator, ", ");
    gen(generator, get_str(mat1->exp_result_type.width));
    gen(generator, ", ");
    generate_expression(generator, mat2);
    gen(generator, ", ");
    gen(generator, get_str(mat2->exp_result_type.height));
    gen(generator, ", ");
    gen(generator, get_str(mat2->exp_result_type.width));
    gen(generator, ")");
}

void generate_sca_mul(Generator *generator, ASTNode *mat, ASTNode *sca)
{
    gen(generator, "mat_sca_mul(");
    generate_expression(generator, mat);
    gen(generator, ", ");
    gen(generator, get_str(mat->exp_result_type.height));
    gen(generator, ", ");
    gen(generator, get_str(mat->exp_result_type.width));
    gen(generator, ", ");
    generate_expression(generator, sca);
    gen(generator, ")");
}

void generate_code_string(Generator *generator)
{
    generate_new_code_string(generator);
    ASTNode *root = generator->tree->root;
    for (int i = 0; i < root->num_contents; i++)
    {
        switch (root->contents[i].type)
        {
        case AST_STMT:
            gen(generator, "\t");
            generate_statement(generator, &root->contents[i]);
            gen(generator, "\n");
            continue;
        case AST_FOR_LOOP:
            generate_for_loop(generator, &root->contents[i]);
            continue;
        default:
            printf("Uncaught error with the parser.\n");
            exit(1);
        }
    }
    gen(generator, "}\n");
}