#pragma once
#include "tokens.h"

typedef struct Scanner
{
  char *contents;
  int line_num;
  int pos;
  int len;

  // Terminated by TKN_EOF
  Token *scanned_tokens;
} Scanner;

void scanner_init(Scanner *scanner, char *contents);
static char advance_chr(Scanner *scanner);
static void go_back_chr(Scanner *scanner);
static char peek_prev_chr(Scanner *scanner);
Token *get_tokens_from_file(char *filename);
Scanner *scan_str(char *str);
Token *get_tokens_with_scanner(Scanner *scanner);
Token *get_token_from_scanner(Scanner *scanner);
Token *get_identifier_or_keyword_from_scanner(Scanner *scanner);
Token *get_number_from_scanner(Scanner *scanner);
Token *get_operator_from_scanner(Scanner *scanner);
int is_operator(char a);
TokenType get_keyword_token(char *a);
static void scanner_exit_with_error(Scanner *scanner, char *error_msg);