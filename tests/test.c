#include <stdio.h>
#include "parser-test.h"
#include "scanner-test.h"

int main()
{
  printf("Running code tests...\n");
  run_scanner_tests();
  run_parser_tests();
  printf("Completed code tests!\n");
}