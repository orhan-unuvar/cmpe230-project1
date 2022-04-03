c_files = parser.c scanner.c symbol-table.c tokens.c code-gen.c

objects = $(c_files:.c=.o)

headers = defs.h $(c_files:.c=.h)

others = preamble.h

matlang2c: $(c_files) $(objects) $(headers) $(others)
	gcc -o matlang2c matlang2c.c $(c_files)

matlang2c_dbg: $(objects) $(headers) $(others)
	gcc -Wall -ggdb -o matlang2c matlang2c.c $(c_files)

tests/test: $(objects) $(headers) $(others) tests/test.c tests/scanner-test.h tests/parser-test.h tests/exit-stub.h
	gcc -ggdb -o tests/test tests/test.c $(c_files)

test: tests/test
	./tests/test
	./tests/compilation/run_tests.bash

.PHONY: test matlang2c_dbg
