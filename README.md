# Matlang2C Transpiler

### Getting Started

To compile the transpiler, run:

```bash
make
```

Then you can run our program and give it your matlang program's path:

```bash
./matlang2c path/to/program.mat
```

This will write the C code output to `path/to/program.c` or print `Error (Line x)` if we encountered a compile-time error.

| Warning | Our preamble definitions are required for the output file to compile |
| ------- | :------------------------------------------------------------------- |

If your matlang program is not in the same directory as our source code, please COPY the preamble definition file to that directory:

```bash
cp preamble.h path/to/program_dir
```

After you have the preamble declarations file in the same directory, make sure to link with the math library when compiling the output program:

| Warning | Output program needs to be linked to the math library |
| ------- | :---------------------------------------------------- |

Example command:

```bash
gcc prog.c -lm -o prog
```

## More

For test documentation, check [test docs](tests/README.md) out.

For architecture and code organization, check [architecture docs](ARCHITECTURE.md) out
