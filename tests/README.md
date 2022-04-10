<div style="text-align: justify">

# Tests

## Test Target

The test target in the Makefile compiles our test code binary and runs both the code tests and the main test bash file (`tests/test.bash`). The test bash file enables _strict mode_ (i.e. exit on error and use of undefined variables), then it runs the compilation and fail tests. If any tests fail, the test target fails altogether.

## Code Tests

We have code-based tests that verify the return values of the scanner and parser for many cases. We don't have such tests for the code-gen since our only requirement for the code-gen is that GCC can compile the output, and we test this in the compilation tests.

### Scanner Tests

We test token generation from some simple strings and verify the integrity of the output by checking the stored strings and types of tokens.

### Parser Tests

We run some simple cases and check if the returned parse tree is correct or not. We also stub the exit function and test the cases where the parser is supposed to emit an error.

## Compilation/Fail Tests

We have 2 bash scripts in the folder `compilation` and `fails`, called `run_tests.bash`. These scripts loop over all `.mat` files in the directory and verify the behavior of our program.

### Behavior of `compilation/run_tests`

1. Loop over all `.mat` files
1. Compile the `.mat` file to C
1. Compile the output C program to `prog` via `gcc {filename} -lm`
1. Run `prog` and compare its output to the provided output in `{filename}.out`

During these steps, we catch any error from gcc or matlang2c and print it to the tester. If any error is caught, we exit with code 1.

### Behavior of `fails/run_tests`

1. Loop over all `.mat` files
1. Run `matlang2c` with the `.mat` file
1. Expect the exit code of `matlang2c` to be 1
1. Compare the error output to the output provided in `{filename}.out`

If the code compiles successfully or we receive a different exit code from matlang2c, we exit with code 1.

# Github Actions CI

The `.github` folder contains our CI action which runs our `test` make target on every commit to master. We use this to verify any regressions in master.

</div>
