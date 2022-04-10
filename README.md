<div style="text-align: justify">

# MatLang2C Transpiler

## Getting Started

To compile the transpiler, run:

```bash
make
```

Then you can run the transpiler and give it your MatLang program's path:

```bash
./matlang2c path/to/program.mat
```

This will write the C code output to `path/to/program.c` or print `Error (Line x)` if encountered a compile-time error.

| Warning | Our preamble definitions are required for the output file to compile. |
| ------- | :------------------------------------------------------------------- |

If your MatLang program is not in the same directory as our source code, please COPY the preamble definition file to that directory:

```bash
cp preamble.h path/to/program_dir
```

After you have the preamble declarations file in the same directory, make sure to link with the math library when compiling the output program:

| Warning | Output program needs to be linked to the math library. |
| ------- | :---------------------------------------------------- |

Example command:

```bash
gcc prog.c -lm -o prog
```

## How MatLang2C Transpiler Works

### Overview

The MatLang2C Transpiler (referred to as ML2C from here after on), consists of 3 major parts:

1. Lexical analyzer, or scanner
2. Syntax analyzer, or parser
3. Target code generator

When ML2C starts to run on a given .mat file input, what happens first is that the scanner *scans* the characters that exists in the .mat file, and *glues* them together to make sensible parts called **tokens**. Then these tokens are passed on to the next part of ML2C, which is the parser, in order to check for any possible syntactical or semantical mistakes (such as type compatibility), and also structure them in a way called an **abstract syntax tree (AST)**, to make it easier to generate the target code before passing it over to the code generator. Finally the code generator *generates* the target C code for an existing C compiler to translate further and produce an executable.

### Scanner

The first major part of ML2C, that is the scanner, simply does a linear traversal of the input MatLang source code and forms tokens out of the characters according to the lexical grammar of the MatLang language. An example of this for integer and float numerals:

<pre>
let c be the next character
if c is a digit
    let t be a new token
    while c is a digit
        add c to t
        let c be the next character
    if c is '.'
        add c to t
        let c be the next character
        if c is not a digit
            exit with error message
        while c is a digit
            add c to t
            let c be the next character
    label t as TOKEN_NUMERAL
    return t
...
</pre>

An important point is that the scanner holds a list of all the keywords of the MatLang language, such as `for`, `vector`, and `printsep`, so whenever it comes across these words in the input, it labels the created token accordingly.

### Parser

Second major part of ML2C, the parser, takes the list of  all tokens created from the input and *parses* it: that is, it checks for any errors in the way tokens follow each other, and reorganizes them into a structure more representative of the meaning of the input program, called an abstract syntax tree (AST) or parse tree<sup>1</sup>. It does all of these according to the *syntactic* (structure of definitions and statements) and *semantic* (type compatibility of expressions) grammar of the MatLang language. Instead of tokens, the parse tree consists of objects called **nodes**. These are objects that generally represent bigger pieces of the input program (for example an AST node may represent a whole `for` loop), and they carry pointers to nodes that represent their subpieces (a `NODE_FOR_LOOP` will carry a pointer to a `NODE_ONE_LINE_STMT` for each substatement in the program loop). Expression nodes also carry data type information for type-checking.  To take a look at how the parser works, let's see how a sequence of tokens that arrives from the scanner may be formed into a little parse subtree:

<pre>
define PARSE_FOR_LOOP
    let n be a new node
    let t be the next token
    if t is TOKEN_KW_FOR
        let t be the next token
        if t is not TOKEN_OPEN_PAREN
            exit with error message
        let t be the next token
        if t is not TOKEN_ID
            exit with error message
        add t to n
        let t be the next token
        if t is not TOKEN_KW_IN
            exit with error message
        for 2 times
            call PARSE_EXPRESSION and add to n
            let t be the next token
            if t is not TOKEN_COLON
                exit with error message
        call PARSE_EXPRESSION and add to n
        let t be the next token
        if t is not TOKEN_CLOSE_PAREN
            exit with error message
        let t be the next token
        if t is not TOKEN_OPEN_BRACE
            exit with error message
        let t be the next token
        if t is not TOKEN_NEW_LINE
            exit with error message
        while next token is not TOKEN_CLOSE_BRACE
            call PARSE_ONE_LINE_STMT and add to n
    return n
</pre>

Two important things to observe here: First of all, any error that is comed across in the subpieces of the `for` loop such as the clause expressions or the substatements will be generated in the subcalls to functions that parse these pieces. Secondly, any time one of these subcalls returns, they return with their corresponding nodes, and then pointers to these nodes are added into the original `for` node.

For another example, let's look at how summation expressions are parsed, so we can see how operator precedence, associativity and type info synthesis handled at once:

<pre>
define 
define PARSE_SUM
    call PARSE_PRODUCT and let it be nl
    while the next token is TOKEN_PLUS or TOKEN_MINUS
        call PARSE_PRODUCT and let it be nr
        let ns be a new node
        if nl and nr are scalars
            label ns scalar
        else if nl and nr are vectors
            if nl.length does not equal nr.length
                exit with error message
            label ns vector
            let ns.length be nl.length
        else if nl and nr are matrices
            if nl.heigth does not equal nr.heigth
                exit with error message
            if nl.width does not equal nr.width
                exit with error message
            label ns matrix
            let ns.heigth be nl.heigth
            let ns.width be nl.width
        else //operand types are not compatible
            exit with error message
        add nl and nr to ns
        let nl be ns
    return nl

define PARSE_PRODUCT
    call PARSE_ATOMIC and let it be nl
    while next token is TOKEN_ASTERISK
        call PARSE_ATOMIC and let it be nr
        ...
    return nl

define PARSE_ATOMIC
    if next token is TOKEN_OPEN_PAREN
        call and return PARSE_EXPRESSION
        if next token is not TOKEN_CLOSE_PAREN
            exit with error message
    else if next token t is TOKEN_ID
        if next token is TOKEN_OPEN_BRACKET
            call and return PARSE_INDEXING with t
            if next token is not TOKEN_CLOSE_BRACKET
                exit with error message
        else return NODE_ID from t
    else if next token is TOKEN_FUNC
    ...
</pre>

As you can see, the precedence is handled by the layerization of precedence levels with different functions, that is 

### Code Generator




## More

For test documentation, check [test docs](tests/README.md) out.

For architecture and code organization, check [architecture docs](ARCHITECTURE.md) out.


<sup>1</sup>The word "parse tree" actually refers to something little different than an abstract syntax tree in the literature, but for the workings and purposes of ML2C, this distinction is irrelevant.

</div>