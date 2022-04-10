<div style="text-align: justify">

# How the MatLang2C Transpiler Works

## Overview

The MatLang2C Transpiler (referred to as ML2C from here after on), consists of 3 major components:

1. The lexical analyzer, or scanner
2. The syntax analyzer, or parser
3. Target code generator

When ML2C starts running on a given .mat file input, the scanner _scans_ the characters that exists in the .mat file, and _glues_ them together to make sensible parts called **tokens**. Then these tokens are passed onto the next component of ML2C, the parser. The parser checks for any possible syntactical or semantical mistakes (such as type compatibility) in the token sequence, and also structures the tokens in an **abstract syntax tree (AST)** to make code generation easier. Finally, the code generator _generates_ C code for an existing C compiler to translate further and produce an executable.

## Scanner

The first major component of ML2C, the scanner, simply does a linear traversal of the input MatLang source code and forms tokens out of the characters according to the lexical grammar of the MatLang language. An example of this for integer and float numerals:

    let ch be the next character
    if ch is a digit
        let tk be a new token
        while ch is a digit
            add ch to tk
            let ch be the next character
        if ch is '.'
            add ch to tk
            let ch be the next character
            if ch is not a digit
                exit with error message
            while ch is a digit
                add ch to tk
                let ch be the next character
        label tk as TOKEN_NUMERAL
        return tk
    ...

An important point is that the scanner holds a list of all the keywords of the MatLang language, such as `for`, `vector`, and `printsep`, so whenever it comes across these words in the input, it labels the created token accordingly.

The scanner is implemented with a struct appropriately called `Scanner`, that is defined in `scanner.h` and used in `scanner.c`. This struct has a pointer both to the source code `char *contents` and the scanned tokens list `Token *scanned_tokens`, and also carries extra information such as the current line number, the current character position in the source code, and the total length of source code.

Some important functions from `scanner.c` are the following:

1. get_tokens_from_file
2. get_tokens_from_scanner
3. get_token_with_scanner

## Parser

Second major component of ML2C, the parser, takes the list of all tokens created from the input and _parses_ it. It checks for any errors in the way tokens follow each other, and reorganizes them into a structure more representative of the meaning of the input program, called an abstract syntax tree (AST) or parse tree<sup>1</sup>. It does this according to the _syntactic_ (structure of definitions and statements) and _semantic_ (type compatibility of expressions) grammar of the MatLang language. Instead of tokens, the parse tree consists of objects called **nodes**. These are objects that generally represent bigger pieces of the input program (for example an AST node may represent a whole `for` loop), and they carry pointers to nodes that represent their subpieces (a `NODE_FOR_LOOP` will carry a pointer to a `NODE_ONE_LINE_STMT` for each substatement in the program loop). Expression nodes also carry data type information for type-checking. To take a look at how the parser works, let's see how a sequence of tokens that arrives from the scanner may be formed into a little parse subtree:

    define PARSE_FOR_LOOP
        let nd be a new node
        let tk be the next token
        if tk is TOKEN_KW_FOR
            let tk be the next token
            if tk is not TOKEN_OPEN_PAREN
                exit with error message
            let tk be the next token
            if tk is not TOKEN_ID
                exit with error message
            add tk to nd
            let tk be the next token
            if tk is not TOKEN_KW_IN
                exit with error message
            for 2 times
                call PARSE_EXPRESSION and add to nd
                let tk be the next token
                if tk is not TOKEN_COLON
                    exit with error message
            call PARSE_EXPRESSION and add to nd
            let tk be the next token
            if tk is not TOKEN_CLOSE_PAREN
                exit with error message
            let tk be the next token
            if tk is not TOKEN_OPEN_BRACE
                exit with error message
            let tk be the next token
            if tk is not TOKEN_NEW_LINE
                exit with error message
            while next token is not TOKEN_CLOSE_BRACE
                call PARSE_ONE_LINE_STMT and add to nd
        return nd

Two important things to observe here: First of all, any error that is comed across in the subpieces of the `for` loop such as the clause expressions or the substatements will be generated in the subcalls to functions that parse these pieces. Secondly, any time one of these subcalls returns, they return with their corresponding nodes, and then pointers to these nodes are added into the original `for` node.

For another example, let's look at how summation expressions are parsed, so we can see how operator precedence, associativity and type info synthesis are handled at once:

    define PARSE_SUM
        call PARSE_PRODUCT and let it be lnd
        while the next token is TOKEN_PLUS or TOKEN_MINUS
            call PARSE_PRODUCT and let it be rnd
            let snd be a new node
            if lnd and rnd are scalars
                label snd as TYPE_SCALAR
            else if lnd and rnd are vectors
                if lnd.length does not equal rnd.length
                    exit with error message
                label snd as TYPE_VECTOR
                let snd.length be lnd.length
            else if lnd and rnd are matrices
                if lnd.heigth does not equal rnd.heigth
                    exit with error message
                if lnd.width does not equal rnd.width
                    exit with error message
                label snd as TYPE_MATRIX
                let snd.heigth be lnd.heigth
                let snd.width be lnd.width
            else //operand types are not compatible
                exit with error message
            add lnd and rnd to snd
            let lnd be snd
        return lnd

    define PARSE_PRODUCT
        call PARSE_ATOMIC and let it be lnd
        while next token is TOKEN_ASTERISK
            call PARSE_ATOMIC and let it be rnd
            ...
        return lnd

    define PARSE_ATOMIC
        if next token is TOKEN_OPEN_PAREN
            call and return PARSE_EXPRESSION
            if next token is not TOKEN_CLOSE_PAREN
                exit with error message
        else if next token tk is TOKEN_ID
            if next token is TOKEN_OPEN_BRACKET
                call and return PARSE_INDEXING with tk
                if next token is not TOKEN_CLOSE_BRACKET
                    exit with error message
            else return NODE_ID from tk
        else if next token tk is TOKEN_NUM
            return NODE_NUM from tk
        else if next token is TOKEN_FUNC_CALL
        ...
        else //meaningless expression
            exit with error message

As you can see, the precedence is handled by the layerization of
precedence
levels with different functions. Whenever the parse enters a place where
it expects a sequence of tokens that make up a meaningful expression, it
recursively descends into first a sum expression, then a product
expression, and finally an atomic expression (which can be either a
parenthesized arbitrary expression, an indexing expression, a single
identifier expression, a single numeral expression, or a function call
expression). The whole expression may be only an atomic expression, or a
bunch of products of atomic expressions, or even a bunch of sums of
products of atomic expressions!

Associativity is also easy, it is simply
achieved by the line `let lnd be snd` in the above pseudocode. What this
allows is that whenever the `while` loop iterates over to add in another
summation operator (that is `+` or `-`) to the expression, it takes the
previous sum node `snd` and stores it as the left node `lnd` of the next
sum node.

Finally the type info synthesis is achieved by a simple type check (that
includes the height and width checks for vectors and matrices) on the
left and right nodes that are already parsed, which is followed by an
assignment of type info on the sum node.

## Code Generator

The final component of ML2C, target code generation, is where all the work
done by the scanner and parser gives its fruit. The code generator simply
does a depth first traversal of the parse tree passed on from the parser,
and generates the target C code along the way.

The Generator struct that represents the code generator has a pointer both
to a `ParseTree tree` and `char [] code_string`. The `code_string` array is
dynamic (as in the sense of a dynamic array), it starts at a length of 100
characters and expands only when its capacity is reached.

Here's an example of how the generator works for variable definitions:

    define GENERATE_DEFINITION (nd)
        if nd.data_type equals TYPE_SCALAR
            GENERATE ("double")
            GENERATE (nd.var_name)
            GENERATE (" = 0;")
        else if nd.data_type is one of TYPE_VECTOR or TYPE_MATRIX
            GENERATE ("double **")
            GENERATE (nd.var_name)
            GENERATE (" = allocate_matrix(")
            GENERATE (nd.height)
            GENERATE (", ")
            GENERATE (nd.width)
            GENERATE (");")

As you can see, both vector and matrices are implemented as double pointers
in ML2C. Also, since C does not have a function to copy 2-dimensional by
default, it is implemented as a custom defined function `allocate_matrix()`. Observe by the way how type information about nodes can be get readily
since they were already synthesized in the parsing phase.

## The Preamble

Since MatLang introduces functions and operations which do not belong to C
by default, such as `tr`, `choose`, or `matrix multiplication` (which is
actually overloaded into the operator `*`), ML2C implements a list of
custom defined functions that achieves the desired behavior. This list of
custom defined functions is stored in a header called `preamble.h`, so
there is a `#include "preamble.h"` instruction at the the start of every
ML2C output program.

| Warning | Our preamble definitions are required for the output file to compile. |
| ------: | --------------------------------------------------------------------- |

If your MatLang program is not in the same directory as our source code,
please COPY the preamble definition file `preamble.h` to that directory:

```bash
cp preamble.h path/to/program_dir
```

Here's a list of functions that exist in `preamble.h`:

1. `allocate_matrix(int height, int width)`
1. `assign_mat_to_mat(double **mat1, double **mat2, int height, int width)`
1. `assign_to_mat(double **, int height, int width, ...)`
1. `mat_add(double **mat1, double **mat2, int height, int width)`
1. `mat_sub(double **mat1, double **mat2, int height, int width)`
1. `mat_mul(double **mat, int height, int width, double **, int height, int width)`
1. `mat_sca_mul(double **mat, int height, int, double)`
1. `tr(double **mat, int height, int width)`
1. `choose(double expr1, double expr2, double expr3, double expr4)`
1. `print(double num)`
1. `print_num(double num)`
1. `print_mat(double **, int height, int width)`
1. `printsep()`
1. `get_int(double scalar)`

<sup>1</sup>The word "parse tree" actually refers to something little different than an abstract syntax tree in the literature, but for the workings and purposes of ML2C, this distinction is irrelevant.

</div>
