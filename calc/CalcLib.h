#ifndef CALCLIB_H
#define CALCLIB_H

#ifdef __cplusplus
extern "C" {
#endif
typedef struct express {
    int type;
    union { double value; const double* bound; const void* function; };
    void* parameters[1];
} express;

enum {
    EX_VARIABLE = 0,

    FUNCTION0 = 8, FUNCTION1, FUNCTION2, FUNCTION3,
    FUNCTION4, FUNCTION5, FUNCTION6, FUNCTION7,

    CLOSURE0 = 16, CLOSURE1, CLOSURE2, CLOSURE3,
    CLOSURE4, CLOSURE5, CLOSURE6, CLOSURE7,

    FLAG_PURE = 32
};
typedef struct variable {
    const char* name;
    const void* address;
    int type;
    void* context;
} variable;
double interpretate(const char* expr, int* error);
express* compile(const char* expression, const variable* variables, int var_count, int* error);
double evaluate(const express* n);
void ex_free(express* n);

#ifdef __cplusplus
}
#endif

#endif