#include <stdio.h>
#include <string.h>
#include "scanner.h"
#include "parser.h"
#include "code-gen.h"
#include "matlang2c.h"

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

   char *filename = convert_file_name(argv[1]);
   FILE *fp;
   fp = fopen(filename, "w");
   fputs(gen->code_string, fp);
   fclose(fp);
   return 0;
}
// Convert abc/cde/file_name.mat to abc/cde/file_name.c
char *convert_file_name(char *file_name)
{
   char *dup = strdup(file_name);
   char *ext = strrchr(dup, '.');
   if (ext == NULL)
   {
      printf("No extension found in filename\n");
      exit(1);
   }
   ext[0] = '\0';
   strcat(dup, ".c");
   return dup;
}