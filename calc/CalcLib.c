#include "CalcLib.h"

#ifndef NAN
    #define NAN (0.0/0.0)
#endif
#ifndef INFINITY
    #define INFINITY (1.0/0.0)
#endif
typedef double (*te_fun2)(double, double);
enum {
    EX_NULL = CLOSURE7+1, ERROR, END, SEP, OPEN, CLOSE, NUMBER, VARIABLE, INFIX
};
enum {CONSTANT = 1};
typedef struct state {
    const char *start;
    const char *next;
    int type;
    union {double value; const double *bound; const void *function;};
    void *context;

    const variable *lookup;
    int lookup_len;
} state;

#define TYPE_MASK(TYPE) ((TYPE)&0x0000001F)
#define IS_PURE(TYPE) (((TYPE) & FLAG_PURE) != 0)
#define IS_FUNCTION(TYPE) (((TYPE) & FUNCTION0) != 0)
#define IS_CLOSURE(TYPE) (((TYPE) & CLOSURE0) != 0)
#define ARITY(TYPE) ( ((TYPE) & (FUNCTION0 | CLOSURE0)) ? ((TYPE) & 0x00000007) : 0 )
#define NEW_EXPR(type, ...) new_expr((type), (const express*[]){__VA_ARGS__})

static express *new_expr(const int type, const express *parameters[]) {
    const int arity = ARITY(type);
    const int psize = sizeof(void*) * arity;
    const int size = (sizeof(express) - sizeof(void*)) + psize + (IS_CLOSURE(type) ? sizeof(void*) : 0);
    express *ret = malloc(size);
    memset(ret, 0, size);
    if (arity && parameters) {
        memcpy(ret->parameters, parameters, psize);
    }
    ret->type = type;
    ret->bound = 0;
    return ret;
}

void free_parameters(express *n) {
    if (!n) return;
    switch (TYPE_MASK(n->type)) {
        case FUNCTION7: case CLOSURE7: free(n->parameters[6]);
        case FUNCTION6: case CLOSURE6: free(n->parameters[5]);
        case FUNCTION5: case CLOSURE5: free(n->parameters[4]);
        case FUNCTION4: case CLOSURE4: free(n->parameters[3]);
        case FUNCTION3: case CLOSURE3: free(n->parameters[2]);
        case FUNCTION2: case CLOSURE2: free(n->parameters[1]);
        case FUNCTION1: case CLOSURE1: free(n->parameters[0]);
    }
}

void free(express *n) {
    if (!n) return;
    free_parameters(n);
    free(n);
}

static double pi(void) {return 3.14159265358979323846;}
static double e(void) {return 2.71828182845904523536;}
static double silnia(double a) {
    if (a < 0.0)
        return NAN;
    if (a > UINT_MAX)
        return INFINITY;
    unsigned int ua = (unsigned int)(a);
    unsigned long int result = 1, i;
    for (i = 1; i <= ua; i++) {
        if (i > ULONG_MAX / result)
            return INFINITY;
        result *= i;
    }
    return (double)result;
}

static const variable functions[] = {
    {"abs", fabs,     FUNCTION1 | FLAG_PURE, 0},
    {"acos", acos,    FUNCTION1 | FLAG_PURE, 0},
    {"asin", asin,    FUNCTION1 | FLAG_PURE, 0},
    {"atan", atan,    FUNCTION1 | FLAG_PURE, 0},
    {"atan2", atan2,  FUNCTION2 | FLAG_PURE, 0},
    {"ceil", ceil,    FUNCTION1 | FLAG_PURE, 0},
    {"cos", cos,      FUNCTION1 | FLAG_PURE, 0},
    {"cosh", cosh,    FUNCTION1 | FLAG_PURE, 0},
    {"e", e,          FUNCTION0 | FLAG_PURE, 0},
    {"exp", exp,      FUNCTION1 | FLAG_PURE, 0},
    {"fac", silnia,   FUNCTION1 | FLAG_PURE, 0},
    {"floor", floor,  FUNCTION1 | FLAG_PURE, 0},
    {"log", log,       FUNCTION1 | FLAG_PURE, 0},
    {"log10", log10,  FUNCTION1 | FLAG_PURE, 0},
    {"pi", pi,        FUNCTION0 | FLAG_PURE, 0},
    {"pow", pow,      FUNCTION2 | FLAG_PURE, 0},
    {"sin", sin,      FUNCTION1 | FLAG_PURE, 0},
    {"sinh", sinh,    FUNCTION1 | FLAG_PURE, 0},
    {"sqrt", sqrt,    FUNCTION1 | FLAG_PURE, 0},
    {"tan", tan,      FUNCTION1 | FLAG_PURE, 0},
    {"tanh", tanh,    FUNCTION1 | FLAG_PURE, 0},
    {0, 0, 0, 0}
};

static const variable *find_builtin(const char *name, int len) {
    int imin = 0;
    int imax = sizeof(functions) / sizeof(variable) - 2;
    while (imax >= imin) {
        const int i = (imin + ((imax-imin)/2));
        int c = strncmp(name, functions[i].name, len);
        if (!c) c = '\0' - functions[i].name[len];
        if (c == 0) {
            return functions + i;
        } else if (c > 0) {
            imin = i + 1;
        } else {
            imax = i - 1;
        }
    }
    return 0;
}

static const variable *find_lookup(const state *s, const char *name, int len) {
    int iters;
    const variable *var;
    if (!s->lookup) return 0;

    for (var = s->lookup, iters = s->lookup_len; iters; ++var, --iters) {
        if (strncmp(name, var->name, len) == 0 && var->name[len] == '\0') {
            return var;
        }
    }
    return 0;
}

static double add(double a, double b) {return a + b;}
static double sub(double a, double b) {return a - b;}
static double mul(double a, double b) {return a * b;}
static double divide(double a, double b) {return a / b;}
static double negate(double a) {return -a;}
static double comma(double a, double b) {(void)a; return b;}

void next_token(state *s) {
    s->type = EX_NULL;
    do {
        if (!*s->next){
            s->type = END;
            return;
        }
        if ((s->next[0] >= '0' && s->next[0] <= '9') || s->next[0] == '.') {
            s->value = strtod(s->next, (char**)&s->next);
            s->type = NUMBER;
        } else {
            if (s->next[0] >= 'a' && s->next[0] <= 'z') {
                const char *start;
                start = s->next;
                while ((s->next[0] >= 'a' && s->next[0] <= 'z') || (s->next[0] >= '0' && s->next[0] <= '9') || (s->next[0] == '_')) s->next++;
                const variable *var = find_lookup(s, start, s->next - start);
                if (!var) var = find_builtin(start, s->next - start);
                if (!var) {
                    s->type = ERROR;
                } else {
                    switch(TYPE_MASK(var->type))
                    {
                        case EX_VARIABLE:
                            s->type = VARIABLE;
                            s->bound = var->address;
                            break;
                        case CLOSURE0: case CLOSURE1: case CLOSURE2: case CLOSURE3:
                        case CLOSURE4: case CLOSURE5: case CLOSURE6: case CLOSURE7:
                            s->context = var->context;
                        case FUNCTION0: case FUNCTION1: case FUNCTION2: case FUNCTION3:
                        case FUNCTION4: case FUNCTION5: case FUNCTION6: case FUNCTION7:
                            s->type = var->type;
                            s->function = var->address;
                            break;
                    }
                }
            } else {
                switch (s->next++[0]) {
                    case '+': s->type = INFIX; s->function = add; break;
                    case '-': s->type = INFIX; s->function = sub; break;
                    case '*': s->type = INFIX; s->function = mul; break;
                    case '/': s->type = INFIX; s->function = divide; break;
                    case '^': s->type = INFIX; s->function = pow; break;
                    case '%': s->type = INFIX; s->function = fmod; break;
                    case '(': s->type = OPEN; break;
                    case ')': s->type = CLOSE; break;
                    case ',': s->type = SEP; break;
                    case ' ': case '\t': case '\n': case '\r': break;
                    default: s->type = ERROR; break;
                }
            }
        }
    } while (s->type == EX_NULL);
}


static express *list(state *s);
static express *expr(state *s);
static express *power(state *s);

static express *base(state *s) {
    express *ret;
    int arity;

    switch (TYPE_MASK(s->type)) {
        case NUMBER:
            ret = new_expr(CONSTANT, 0);
            ret->value = s->value;
            next_token(s);
            break;

        case VARIABLE:
            ret = new_expr(EX_VARIABLE, 0);
            ret->bound = s->bound;
            next_token(s);
            break;

        case FUNCTION0:
        case CLOSURE0:
            ret = new_expr(s->type, 0);
            ret->function = s->function;
            if (IS_CLOSURE(s->type)) ret->parameters[0] = s->context;
            next_token(s);
            if (s->type == OPEN) {
                next_token(s);
                if (s->type != CLOSE) {
                    s->type = ERROR;
                } else {
                    next_token(s);
                }
            }
            break;

        case FUNCTION1:
        case CLOSURE1:
            ret = new_expr(s->type, 0);
            ret->function = s->function;
            if (IS_CLOSURE(s->type)) ret->parameters[1] = s->context;
            next_token(s);
            ret->parameters[0] = power(s);
            break;

        case FUNCTION2: case FUNCTION3: case FUNCTION4:
        case FUNCTION5: case FUNCTION6: case FUNCTION7:
        case CLOSURE2: case CLOSURE3: case CLOSURE4:
        case CLOSURE5: case CLOSURE6: case CLOSURE7:
            arity = ARITY(s->type);

            ret = new_expr(s->type, 0);
            ret->function = s->function;
            if (IS_CLOSURE(s->type)) ret->parameters[arity] = s->context;
            next_token(s);

            if (s->type != OPEN) {
                s->type = ERROR;
            } else {
                int i;
                for(i = 0; i < arity; i++) {
                    next_token(s);
                    ret->parameters[i] = expr(s);
                    if(s->type != SEP) {
                        break;
                    }
                }
                if(s->type != CLOSE || i != arity - 1) {
                    s->type = ERROR;
                } else {
                    next_token(s);
                }
            }

            break;

        case OPEN:
            next_token(s);
            ret = list(s);
            if (s->type != CLOSE) {
                s->type = ERROR;
            } else {
                next_token(s);
            }
            break;

        default:
            ret = new_expr(0, 0);
            s->type = ERROR;
            ret->value = NAN;
            break;
    }

    return ret;
}


static express *power(state *s) {
    int sign = 1;
    while (s->type == INFIX && (s->function == add || s->function == sub)) {
        if (s->function == sub) sign = -sign;
        next_token(s);
    }
    express *ret;
    if (sign == 1) {
        ret = base(s);
    } else {
        ret = NEW_EXPR(FUNCTION1 | FLAG_PURE, base(s));
        ret->function = negate;
    }
    return ret;
}

static express *factor(state *s) {
    express *ret = power(s);
    while (s->type == INFIX && (s->function == pow)) {
        te_fun2 t = s->function;
        next_token(s);
        ret = NEW_EXPR(FUNCTION2 | FLAG_PURE, ret, power(s));
        ret->function = t;
    }
    return ret;
}



static express *term(state *s) {
    express *ret = factor(s);
    while (s->type == INFIX && (s->function == mul || s->function == divide || s->function == fmod)) {
        te_fun2 t = s->function;
        next_token(s);
        ret = NEW_EXPR(FUNCTION2 | FLAG_PURE, ret, factor(s));
        ret->function = t;
    }
    return ret;
}


static express *expr(state *s) {
    express *ret = term(s);

    while (s->type == INFIX && (s->function == add || s->function == sub)) {
        te_fun2 t = s->function;
        next_token(s);
        ret = NEW_EXPR(FUNCTION2 | FLAG_PURE, ret, term(s));
        ret->function = t;
    }
    return ret;
}


static express *list(state *s) {
    express *ret = expr(s);
    while (s->type == SEP) {
        next_token(s);
        ret = NEW_EXPR(FUNCTION2 | FLAG_PURE, ret, expr(s));
        ret->function = comma;
    }
    return ret;
}

#define FUN(...) ((double(*)(__VA_ARGS__))n->function)
#define M(e) evaluate(n->parameters[e])

double evaluate(const express *n) {
    if (!n) return NAN;

    switch(TYPE_MASK(n->type)) {
        case CONSTANT: return n->value;
        case EX_VARIABLE: return *n->bound;

        case FUNCTION0: case FUNCTION1: case FUNCTION2: case FUNCTION3:
        case FUNCTION4: case FUNCTION5: case FUNCTION6: case FUNCTION7:
            switch(ARITY(n->type)) {
                case 0: return FUN(void)();
                case 1: return FUN(double)(M(0));
                case 2: return FUN(double, double)(M(0), M(1));
                case 3: return FUN(double, double, double)(M(0), M(1), M(2));
                case 4: return FUN(double, double, double, double)(M(0), M(1), M(2), M(3));
                case 5: return FUN(double, double, double, double, double)(M(0), M(1), M(2), M(3), M(4));
                case 6: return FUN(double, double, double, double, double, double)(M(0), M(1), M(2), M(3), M(4), M(5));
                case 7: return FUN(double, double, double, double, double, double, double)(M(0), M(1), M(2), M(3), M(4), M(5), M(6));
                default: return NAN;
            }

        case CLOSURE0: case CLOSURE1: case CLOSURE2: case CLOSURE3:
        case CLOSURE4: case CLOSURE5: case CLOSURE6: case CLOSURE7:
            switch(ARITY(n->type)) {
                case 0: return FUN(void*)(n->parameters[0]);
                case 1: return FUN(void*, double)(n->parameters[1], M(0));
                case 2: return FUN(void*, double, double)(n->parameters[2], M(0), M(1));
                case 3: return FUN(void*, double, double, double)(n->parameters[3], M(0), M(1), M(2));
                case 4: return FUN(void*, double, double, double, double)(n->parameters[4], M(0), M(1), M(2), M(3));
                case 5: return FUN(void*, double, double, double, double, double)(n->parameters[5], M(0), M(1), M(2), M(3), M(4));
                case 6: return FUN(void*, double, double, double, double, double, double)(n->parameters[6], M(0), M(1), M(2), M(3), M(4), M(5));
                case 7: return FUN(void*, double, double, double, double, double, double, double)(n->parameters[7], M(0), M(1), M(2), M(3), M(4), M(5), M(6));
                default: return NAN;
            }

        default: return NAN;
    }
}

#undef FUN
#undef M

static void optimize(express* n) {
    if (n->type == CONSTANT) return;
    if (n->type == EX_VARIABLE) return;
    if (IS_PURE(n->type)) {
        const int arity = ARITY(n->type);
        int known = 1;
        int i;
        for (i = 0; i < arity; ++i) {
            optimize(n->parameters[i]);
            if (((express*)(n->parameters[i]))->type != CONSTANT) {
                known = 0;
            }
        }
        if (known) {
            const double value = evaluate(n);
            free_parameters(n);
            n->type = CONSTANT;
            n->value = value;
        }
    }
}

express *compile(const char *expression, const variable *variables, int var_count, int *error) {
    state s;
    s.start = s.next = expression;
    s.lookup = variables;
    s.lookup_len = var_count;

    next_token(&s);
    express *root = list(&s);

    if (s.type != END) {
        free(root);
        if (error) {
            *error = (s.next - s.start);
            if (*error == 0) *error = 1;
        }
        return 0;
    } else {
        optimize(root);
        if (error) *error = 0;
        return root;
    }
}

double interpretate(const char *expression, int *error) {
    express *n = compile(expression, 0, 0, error);
    double ret;
    if (n) {
        ret = evaluate(n);
        free(n);
    } else {
        ret = NAN;
    }
    return ret;
}

static void pn (const express *n, int depth) {
    int i, arity;
    printf("%*s", depth, "");

    switch(TYPE_MASK(n->type)) {
    case CONSTANT: printf("%f\n", n->value); break;
    case EX_VARIABLE: printf("bound %p\n", n->bound); break;

    case FUNCTION0: case FUNCTION1: case FUNCTION2: case FUNCTION3:
    case FUNCTION4: case FUNCTION5: case FUNCTION6: case FUNCTION7:
    case CLOSURE0: case CLOSURE1: case CLOSURE2: case CLOSURE3:
    case CLOSURE4: case CLOSURE5: case CLOSURE6: case CLOSURE7:
         arity = ARITY(n->type);
         printf("f%d", arity);
         for(i = 0; i < arity; i++) {
             printf(" %p", n->parameters[i]);
         }
         printf("\n");
         for(i = 0; i < arity; i++) {
             pn(n->parameters[i], depth + 1);
         }
         break;
    }
}