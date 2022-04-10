<div style="text-align: justify">

# MatLang2C Transpiler

> Note:
> We recommend using a Markdown previewer when reading these docs. We prefer
> VS Code's Preview mode. (Cmd+Shift+V on Mac, Ctrl+Shift+V on Windows)

## Getting Started

To compile the transpiler, run:

```bash
make
```

Then you can run the transpiler and give it your MatLang program's path:

```bash
./matlang2c path/to/program.mat
```

This will write the C code output to `path/to/program.c` or print `Error (Line x)` if a compile-time error occurs.

| Warning | Our preamble definitions are required for the output file to compile. |
| ------- | :-------------------------------------------------------------------- |

If your MatLang program is not in the same directory as our source code, please COPY the preamble definition file `preamble.h` to that directory:

```bash
cp preamble.h path/to/program_dir
```

After you have the preamble declarations file in the same directory, make sure to link with the math library when compiling the output program:

| Warning | Output program needs to be linked to the math library. |
| ------- | :----------------------------------------------------- |

Example command:

```bash
gcc prog.c -lm -o prog
```

## Running Tests

To run the tests:

    make test

## Next Steps

The docs don't end here. We have docpages explaining our testing suite and the
code structure. Please check them out.

For test documentation, check [test docs](tests/README.md) out.

For architecture and code organization, check [architecture docs](ARCHITECTURE.md) out.

For the structure and nesting of ASTNodes in the parser, check [parser diagrams](PARSER_DIAGRAMS.md) out.

</div>
