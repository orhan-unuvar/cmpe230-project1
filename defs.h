#pragma once

#define N 1000
#define LINE_LIMIT 256
#define ERR_DETAIL 0

// When set to 1, no errors are printed on failure
// Used in our testing suite where we want to catch exits and don't want error
// messages leaking into stdout
extern int SUPPRESS_ALL_ERRS;