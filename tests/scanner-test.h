#include <assert.h>
#include <stdio.h>
#include <string.h>

#include "../tokens.h"
#include "../scanner.h"

void run_scanner_tests();
void scan_challenge_1();
void scan_challenge_2();
void scan_challenge_3();
void scan_challenge_4();

void run_scanner_tests()
{
  printf("Running scanner tests...\n");
  scan_challenge_1();
  scan_challenge_2();
  scan_challenge_3();
  scan_challenge_4();
  printf("Scanner scanner passed!\n");
}

void scan_challenge_1()
{
  char *challenge_1 = "\t\tscalar a";

  Scanner *scanner = scan_str(challenge_1);

  Token *tokens = scanner->scanned_tokens;

  assert(tokens[0].type == TKN_TYPE_SCALAR);
  assert(tokens[1].type == TKN_IDENT);
  assert(strcmp(tokens[1].contents, "a") == 0);
}

void scan_challenge_2()
{
  char *challenge_1 = "for ( \t\t _i in 5: 3:  321) {\n";

  Scanner *scanner = scan_str(challenge_1);

  Token *tokens = scanner->scanned_tokens;

  assert(tokens[0].type == TKN_KW_FOR);

  assert(tokens[1].type == TKN_PN_OPENPAREN);

  assert(tokens[2].type == TKN_IDENT);
  assert(strcmp(tokens[2].contents, "_i") == 0);

  assert(tokens[3].type == TKN_KW_IN);

  assert(tokens[4].type == TKN_INT_LITERAL);

  assert(strcmp(tokens[4].contents, "5") == 0);
  assert(tokens[5].type == TKN_PN_COLON);

  assert(tokens[6].type == TKN_INT_LITERAL);
  assert(strcmp(tokens[6].contents, "3") == 0);

  assert(tokens[7].type == TKN_PN_COLON);

  assert(tokens[8].type == TKN_INT_LITERAL);
  assert(strcmp(tokens[8].contents, "321") == 0);

  assert(tokens[9].type == TKN_PN_CLOSEPAREN);

  assert(tokens[10].type == TKN_PN_OPENBRACE);

  assert(tokens[11].type == TKN_LINE_FEED);

  assert(tokens[12].type == TKN_EOF);
}

void scan_challenge_3()
{
  char *challenge_1 = "\tA={0.5 0.5\t 0 10}\n";

  Scanner *scanner = scan_str(challenge_1);

  Token *tokens = scanner->scanned_tokens;

  assert(tokens[0].type == TKN_IDENT);
  assert(strcmp(tokens[0].contents, "A") == 0);

  assert(tokens[1].type == TKN_ASSIGN);

  assert(tokens[2].type == TKN_PN_OPENBRACE);

  assert(tokens[3].type == TKN_FLOAT_LITERAL);
  assert(strcmp(tokens[3].contents, "0.5") == 0);

  assert(tokens[4].type == TKN_FLOAT_LITERAL);
  assert(strcmp(tokens[4].contents, "0.5") == 0);

  assert(tokens[5].type == TKN_INT_LITERAL);
  assert(strcmp(tokens[5].contents, "0") == 0);

  assert(tokens[6].type == TKN_INT_LITERAL);
  assert(strcmp(tokens[6].contents, "10") == 0);

  assert(tokens[7].type == TKN_PN_CLOSEBRACE);

  assert(tokens[8].type == TKN_LINE_FEED);
}

void scan_challenge_4()
{
  char *challenge = "\n\
  # this program computes fibonacci\n\
  # numbers\n\
  # variable definitions\n\
  scalar i\n\
  scalar n\n\
  vector x[2]\n\
  vector y[2]\n\
  matrix A[2,2]\n\
  matrix B[2,2]\n\
  # statements\n\
  n = 10 \n\
  x={1 1} \n\
  A={1 1 1 0} \n\
  B={1 0  0 1} \n\
  print(x)\n\
  for(i in 1:n:1) {\n\
    B = A*B\n\
    y = B*x\n\
    print(y[1])\n\
  }\n\
  ";
  Scanner *scanner = scan_str(challenge);

  Token *tokens = scanner->scanned_tokens;
}