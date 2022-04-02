#include <stdio.h>
#include "scanner.h"
#include "parser.h"
#include "generator.h"

int main(int argc, char *argv[])
{
   if (argc != 2)
   {

      printf("Give filename as command line argument\n");

      return (1);
   }
   Token *tokens = get_tokens_from_file(argv[1]);
   Parser *parser = new_parser(tokens);
   parse_root(parser, parser->parse_tree->root);

   Generator *gen = new_generator(parser->parse_tree);
   generate_code_string(gen);

   printf("%s", gen->code_string);
}
