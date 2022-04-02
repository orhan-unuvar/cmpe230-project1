c_files = parser.c scanner.c symbol-table.c tokens.c generator.c

objects = $(c_files:.c=.o)

headers = defs.h $(c_files:.c=.h)

others = preamble.c

matlang2c: $(objects) $(headers) $(others)
	cc -o matlang2c matlang2c.c $(c_files)

tests/test: $(objects) $(headers) $(others) tests/test.c tests/scanner-test.h tests/parser-test.h tests/exit-stub.h
	cc -ggdb -o tests/test tests/test.c $(c_files)

test: tests/test
	./tests/test

.PHONY: test
