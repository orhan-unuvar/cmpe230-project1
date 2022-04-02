#include <assert.h>
#include <stdio.h>
#include <string.h>

#include "exit-stub.h"
#include "../tokens.h"
#include "../scanner.h"
#include "../parser.h"
#include "../code-gen.h"

void run_codegen_tests();
void codegen_challenge_1();

void run_codegen_tests()
{
  printf("Running codegen tests...\n");
  codegen_challenge_1();
  printf("Parser codegen passed!\n");
}

char *return_c_code(char *matlang_code)
{
  Token *tokenList = scan_str(matlang_code)->scanned_tokens;
  Parser *parser = new_parser(tokenList);
  parser->parse_tree->root = parse_root(parser, parser->parse_tree->root);
  Generator *generator = new_generator(parser->parse_tree);
  generate_code_string(generator);
  char *code = generator->code_string;
  return code;
}

void codegen_challenge_1()
{
  char *challenge = "vector A[1]";
  char *result = return_c_code(challenge);
  char *expected = "double A[1][1];";

  assert(strcmp(result, expected) == 0);
}

void codegen