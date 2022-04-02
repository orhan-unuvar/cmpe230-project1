#include <code-gen.h>
#include <defs.h>
#include <string.h>

Generator *new_generator(ParseTree *tree)
{
    Generator *generator = calloc(1, sizeof(Generator));
    generator->tree = tree;
    generator->code_string = calloc(1, 10*LINE_LIMIT*LINE_LIMIT); //not sure
    char *string = new_code_string();
    strcpy(generator->code_string, string);
    generator->current = strlen(string);
}

char *new_code_string()
{//not sure
    return strcat(preamble, "#include <stdio.h>\n\n\
            void main()\n\
            {\n\
            ");
}

char *get_str(int val)
{
    char str[LINE_LIMIT];
    sprintf(str, "%d", val);
    return str;
}

void generate_str(Generator *generator, char *string)
{
    strcat(generator->code_string, string);
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
        generate_str(generator, "double ");
        generate_str(generator, node->var_name);
        generate_str(generator, ";");
    }
    else if (node->var_type.var_type == TYPE_VECTOR)
    {
        generate_str(generator, "double ");
        generate_str(generator, node->var_name);
        generate_str(generator, "[");
        generate_str(generator, get_str(node->var_type.height));
        generate_str(generator, "];");
    }
    else if (node->var_type.var_type == TYPE_MATRIX)
    {
        generate_str(generator, "double ");
        generate_str(generator, node->var_name);
        generate_str(generator, "[");
        gnerate_str(generator, get_str(node->var_type.height));
        generate_str(generator, "]");
        generate_str(generator, "[");
        generate_str(generator, get_str(node->var_type.width));
        generate_str("];", generator);
    }
}

void generate_assignment_stmt(Generator *generator, ASTNode *node)
{
    generate_assignment_dest(generator, node->lhs);
    generate_str(generator, " = ");
    generate_expression(generator, node->rhs);
    generate_str(generator, ";");
}

void generate_assignment_dest(Generator *generator, ASTNode *node)
{
    generate_str(generator, node->var_name);
    if (node->is_indexed_assignment)
    {
        generate_str(generator, "[");
        generate_str(generator, node->index_1 - 1); /* might also be expression */
        generate_str(generator, "][");
        if (node->index_2)
        {
            generate_str(generator, node->index_2 - 1); /* might also be expression */
        }
        else
        {
            generate_str(generator, "0");
        }
        generate_str(generator, "]");
    }
}

void generate_print_stmt(Generator *generator, ASTNode *node)
{
    generate_str(generator, "print(");
    generate_expression(generator, node->contents);
    generate_str(generator, ");");
}

void generate_expression(Generator *generator, ASTNode *node)
{
    switch (node->exp_type)
    {
        case EXP_LITERAL:
            generate_str(generator, node->literal_str);
            break;
        case EXP_LIST:
            generate_str(generator, "{");
            for (int i = 0 ; i < node->num_contents - 1 ; i++)
            {
                generate_str(generator, (&node->contents[i])->literal_str);
                generate_str(generator, ",");
            }
            generate_str(generator, &node->contents[node->num_contents - 1]);
            generate_str(generator, "}");
            break;
        case EXP_IDENT:
            generate_str(generator, node->ident);
            break;
        case EXP_BINOP:
            generate_str(generator, "(");
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
            generate_str(generator, ")");
            break;
        case EXP_FUNC_CALL:
            generate_str(generator, node->ident);
            generate_str(generator, "(");
            for (int i = 0; i < node->num_contents - 1; i++)
            {
                generate_str(generator, &node->contents[i]);
                generate_str(generator, ",");
            }
            generate_str(generator, &node->contents[node->num_contents - 1]);
            generate_str(generator, ")");
        case EXP_INDEX:
            generate_str(generator, node->ident);
            if (node->num_contents == 1)
            {
                generate_str(generator, "[get_int(");
                generate_expression(generator, node->contents);
                generate_str(generator, ") - 1][0]");
            }
            else if (node->num_contents == 2)
            {
                generate_str(generator, "[get_int(");
                generate_expression(generator, node->contents);
                generate_str(generator, ") - 1][get_int(");
                generate_expression(generator, &node->contents[1]);
                generate_expression(generator, ") - 1]");
            }
            break;
    }
}

void generate_for_loop(Generator *generator, ASTNode *node)
{
    generate_str(generator, "\t");
    generate_for_clause(generator, node->for_clause_1);
    generate_str(generator, "\n{\n");
    if (node->for_clause_2 != NULL)
    {
        generate_str(generator, "\t\t");
        generate_for_clause(generator, node->for_clause_2);
        generate_str(generator, "\n{\n");
    }
    for (int i = 0 ; i < node->num_contents ; i++)
    {
        generate_str(generator, "\t\t");
        if (node->for_clause_2 != NULL)
        {
            generate_str(generator, "\t");
        }
        generate_statement(generator, node);
        generate_str(generator, "\n");
    }
    if (node->for_clause_2 != NULL)
    {
        generate_str(generator, "\t\t}\n");
    }
    generate_str(generator, "\t}\n");
}

void generate_for_clause(Generator *generator, ForLoopClause *for_clause)
{
    char *var_name = for_clause->var->name;
    generate_str(generator, "for (");
    generate_str(generator, var_name);
    generate_str(generator, " = ");
    generate_expression(generator, for_clause->expr1);
    generate_str(generator, "; ");
    generate_str(generator, "get_int(");
    generate_str(generator, var_name);
    generate_str(generator, ") <= get(");
    generate_expression(generator, for_clause->expr2);
    generate_str(generator, "); ");
    generate_str(generator, var_name);
    generate_str(generator, " += ");
    generate_expression(generator, for_clause->expr3);
    generate_str(generator, ")");
}

void generate_mat_add_sub(Generator *generator, ASTNode *mat1, ASTNode *mat2, char *func_name)
{
    generate_str(generator, func_name);
    generate_str(generator, "(");
    generate_expression(generator, mat1);
    generate_str(generator, ", ");
    generate_expression(generator, mat2);
    generate_str(generator, ", ");
    generate_str(generator, mat1->exp_result_type.height);
    generate_str(generator, ", ");
    generate_str(generator, mat1->exp_result_type.width);
    generate_str(generator, ")");
}

void generate_ord_op(Generator *generator, ASTNode *sca1, ASTNode *sca2, char *chr)
{
    generate_expression(generator, sca1);
    generate_str(generator, chr);
    generate_expression(generator, sca2);
}

void generate_mat_mul(Generator *generator, ASTNode *mat1, ASTNode *mat2)
{
    generate_str(generator, "mat_mul(");
    generate_expression(generator, mat1);
    generate_str(generator, ", ");
    generate_str(generator, mat1->exp_result_type.height);
    generate_str(generator, ", ");
    generate_str(generator, mat1->exp_result_type.width);
    generate_str(generator, ", ");
    generate_expression(generator, mat2);
    generate_str(generator, ", ");
    generate_str(generator, mat2->exp_result_type.height);
    generate_str(generator, ", ");
    generate_str(generator, mat2->exp_result_type.width);
    generate_str(generator, ", ");
    generate_str(generator, ")");
}

void generate_sca_mul(Generator *generator, ASTNode *mat, ASTNode *sca)
{
    generate_str(generator, "mat_sca_mul(");
    generate_expression(generator, mat);
    generate_str(generator, ", ");
    generate_str(generator, mat->exp_result_type.height);
    generate_str(generator, ", ");
    generate_str(generator, mat->exp_result_type.width);
    generate_str(generator, ", ");
    generate_expression(generator, sca);
    generate_str(generator, ")");
}

void generate_code_string(Generator *generator)
{
    ASTNode *root = generator->tree->root;
    for (int i = 0; i < root->num_contents; i++)
    {
        switch (root->contents[i].type)
        {
        case AST_STMT:
            generate_str(generator, "\t");
            generate_statement(generator, &root->contents[i]);
            generate_str(generator, "\n");
        case AST_FOR_LOOP:
            generate_for_loop(generator, &root->contents[i]);
        }  
    }
}