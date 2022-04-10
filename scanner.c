#include <stdio.h>
#include <string.h>
#include <ctype.h>

#include "defs.h"
#include "scanner.h"

// Initializes a scanner with the given contents
void scanner_init(Scanner *scanner, char *contents)
{
  scanner->contents = contents;
  scanner->line_num = 1;
  scanner->pos = 0;
  scanner->len = strlen(contents);
}

// Goes one character forward
static char advance_chr(Scanner *scanner)
{
  scanner->pos++;
  if (scanner->pos - 1 >= scanner->len)
  {
    return '\0';
  }
  char c = scanner->contents[scanner->pos - 1];
  return c;
}

// Goes one character back
static void go_back_chr(Scanner *scanner)
{
  if (scanner->pos == 0)
  {
    return;
  }
  scanner->pos--;
}

// Returns the previous character
static char peek_prev_chr(Scanner *scanner)
{
  if (scanner->pos - 1 < 0)
  {
    return '\0';
  }
  return scanner->contents[scanner->pos - 1];
}

// Receives a filename and scans the file for tokens
// The returned Token array is terminated by a TKN_EOF token
Token *get_tokens_from_file(char *filename)
{
  FILE *fp;
  fp = fopen(filename, "r");

  if (fp == NULL)
  {
    printf("Cannot open %s\n", filename);

    return NULL;
  }
  char line[256];
  char contents[LINE_LIMIT * LINE_LIMIT];

  int line_num = 1;
  while (fgets(line, LINE_LIMIT, fp) != NULL)
  {
    strcat(contents, line);
    line_num++;
  }
  fclose(fp);

  Scanner *scanner = scan_str(contents);
  Token *tokens = scanner->scanned_tokens;
  free(scanner);
  return tokens;
}

// Scan tokens from a string.
Scanner *scan_str(char *str)
{
  Scanner *scanner = calloc(1, sizeof(Scanner));
  scanner_init(scanner, str);
  scanner->scanned_tokens = get_tokens_with_scanner(scanner);

  return scanner;
}

// Gets all tokens until EOF
Token *get_tokens_with_scanner(Scanner *scanner)
{
  Token *tokens = calloc(strlen(scanner->contents), sizeof(Token));
  int tokens_index = 0;

  while (scanner->pos < scanner->len)
  {
    Token *token = get_token_from_scanner(scanner);

    if (token->type == TKN_LINE_FEED)
    {
      scanner->line_num++;
    }
    if (token->type == TKN_EOF)
    {
      break;
    }
    tokens[tokens_index] = *token;
    free(token);
    tokens_index++;
  }
  tokens[tokens_index] = *new_token(TKN_EOF, "", scanner->line_num);

  return tokens;
}

// Gets one token
Token *get_token_from_scanner(Scanner *scanner)
{
  char c = advance_chr(scanner);
  if (c == '\0')
    return new_token(TKN_EOF, strdup(""), scanner->line_num);
  int is_in_comment = 0;
  while (1)
  {
    if (is_in_comment)
    {
      if (c == '\n')
        return new_token(TKN_LINE_FEED, strdup("\n"), scanner->line_num);
      else if (c == '\0')
      {
        return new_token(TKN_EOF, strdup(""), scanner->line_num);
      }
      else
      {
        c = advance_chr(scanner);
        continue;
      }
    }

    switch (c)
    {
    case ' ':
    case '\t':
      break;
    case '\n':
      return new_token(TKN_LINE_FEED, strdup("\n"), scanner->line_num);
    case ',':
      return new_token(TKN_PN_COMMA, strdup(","), scanner->line_num);
    case '(':
      return new_token(TKN_PN_OPENPAREN, strdup("("), scanner->line_num);
    case ')':
      return new_token(TKN_PN_CLOSEPAREN, strdup(")"), scanner->line_num);
    case '{':
      return new_token(TKN_PN_OPENBRACE, strdup("{"), scanner->line_num);
    case '}':
      return new_token(TKN_PN_CLOSEBRACE, strdup("}"), scanner->line_num);
    case '[':
      return new_token(TKN_PN_OPENBRACKET, strdup("["), scanner->line_num);
    case ']':
      return new_token(TKN_PN_CLOSEBRACKET, strdup("]"), scanner->line_num);
    case ':':
      return new_token(TKN_PN_COLON, strdup(":"), scanner->line_num);
    case '=':
      return new_token(TKN_ASSIGN, strdup("="), scanner->line_num);
    case '\0':
      return new_token(TKN_EOF, strdup(""), scanner->line_num);
    case '#':
      is_in_comment = 1;
      break;
    default:
      if (isalpha(c) || c == '_')
      {
        return get_identifier_or_keyword_from_scanner(scanner);
      }
      else if (isdigit(c))
      {
        return get_number_from_scanner(scanner);
      }
      else if (is_operator(c))
      {
        return get_operator_from_scanner(scanner);
      }
      else
      {
        scanner_exit_with_error(scanner, "Unknown character");
      }
    }

    // In case we got whitespace or we're inside a comment, keep on scanning
    c = advance_chr(scanner);
  }
}

// Reads a C-identifier string and returns an identifier
// or a keyword token
Token *get_identifier_or_keyword_from_scanner(Scanner *scanner)
{
  char ident[LINE_LIMIT];

  char c = peek_prev_chr(scanner);
  int i = 0;

  while (isalnum(c) || c == '_')
  {
    ident[i] = c;

    i++;
    c = advance_chr(scanner);
  }
  go_back_chr(scanner);
  ident[i] = '\0';

  TokenType type;
  if ((type = get_keyword_token(ident)) != TKN_UNKNOWN)
  {
    return new_token(type, strdup(ident), scanner->line_num);
  }
  return new_token(TKN_IDENT, strdup(ident), scanner->line_num);
}

// Reads a number literal and returns a literal token
Token *get_number_from_scanner(Scanner *scanner)
{
  char number[LINE_LIMIT];

  char c = peek_prev_chr(scanner);
  int i = 0;

  int is_float = 0;

  while (isdigit(c))
  {
    number[i] = c;

    i++;
    c = advance_chr(scanner);
  }
  if (c == '.')
  {
    is_float = 1;
    number[i] = c;
    i++;
    c = advance_chr(scanner);
    if (!isdigit(c))
    {
      scanner_exit_with_error(scanner, "Expected a digit after .");
    }
    while (isdigit(c))
    {
      number[i] = c;
      i++;
      c = advance_chr(scanner);
    }
  }
  go_back_chr(scanner);

  number[i] = '\0';
  return new_token(is_float ? TKN_FLOAT_LITERAL : TKN_INT_LITERAL, strdup(number), scanner->line_num);
}

// Reads an operator and returns the correct operator Token
Token *get_operator_from_scanner(Scanner *scanner)
{
  char c = peek_prev_chr(scanner);
  TokenType type;
  switch (c)
  {
  case '*':
    type = TKN_MULT;
    break;
  case '+':
    type = TKN_PLUS;
    break;
  case '-':
    type = TKN_MINUS;
    break;
  default:
    return new_token(TKN_UNKNOWN, NULL, scanner->line_num);
  }
  char op[2] = {c, '\0'};
  return new_token(type, strdup(op), scanner->line_num);
}

// Checks whether a character is a valid operator
int is_operator(char a)
{
  switch (a)
  {
  case '*':
  case '+':
  case '-':
    return 1;
  default:
    return 0;
  }
}

// Used in conjunction with KEYWORDS to check whether a string is a keyword
// and if so, return the corresponding TokenType
static const TokenType KEYWORD_TYPES[] = {TKN_KW_FOR, TKN_KW_IN, TKN_TYPE_SCALAR, TKN_TYPE_VECTOR, TKN_TYPE_MATRIX, TKN_SPECIAL_PRINT, TKN_SPECIAL_PRINTSEP, TKN_FUNCTION_TR, TKN_FUNCTION_CHOOSE, TKN_FUNCTION_SQRT};

static const char *KEYWORDS[] = {"for", "in", "scalar", "vector", "matrix", "print", "printsep", "tr", "choose", "sqrt"};

// Returns the corresponding TokenType for a keyword or
// TKN_UNKNOWN if it's not a keyword
TokenType get_keyword_token(char *a)
{
  for (int i = 0; i < sizeof(KEYWORDS) / sizeof(char *); i++)
  {
    if (strcmp(a, KEYWORDS[i]) == 0)
    {
      return KEYWORD_TYPES[i];
    }
  }
  return TKN_UNKNOWN;
}

// Exits the program due to a scanner error
static void scanner_exit_with_error(Scanner *scanner, char *error_msg)
{
  if (SUPPRESS_ALL_ERRS)
    exit(1);
  if (ERR_DETAIL)
    printf("Error (Line %d): %s\n", scanner->line_num, error_msg);
  else
    printf("Error (Line %d)\n", scanner->line_num);
}
