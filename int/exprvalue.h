/*
 * exprvalue.h — dBase III Plus expression value type system
 *
 * Represents a typed runtime value: integer, real, string, date, logical, or null.
 * Provides constructors, comparison, coercion, and formatting helpers.
 */

#ifndef _EXPRVALUE_H
#define _EXPRVALUE_H

#include <stddef.h>

/* ------------------------------------------------------------------ */
/* Value types                                                         */
/* ------------------------------------------------------------------ */

typedef enum {
    VAL_NULL,       /* Null / undefined                               */
    VAL_INTEGER,    /* Integer (stored as double for uniformity)      */
    VAL_REAL,       /* Float/double                                   */
    VAL_STRING,     /* String (heap-allocated, must be freed)         */
    VAL_DATE,       /* Date stored as string "YYYY-MM-DD"             */
    VAL_LOGICAL,    /* Boolean (.T. = 1, .F. = 0)                     */
} ValType;

/* ------------------------------------------------------------------ */
/* ExprValue — discriminated union                                     */
/* ------------------------------------------------------------------ */

typedef struct {
    ValType type;
    union {
        double rval;          /* Numeric (integer or real)            */
        char  *sval;          /* String (malloc'd, NULL for empty)    */
        char   dval[12];      /* Date as "YYYY-MM-DD\0"               */
    } data;
} ExprValue;

/* ------------------------------------------------------------------ */
/* Constructors — return a properly initialized ExprValue              */
/* ------------------------------------------------------------------ */

ExprValue val_null(void);
ExprValue val_integer(int i);
ExprValue val_real(double d);
ExprValue val_string(const char *s);   /* copies s into malloc'd buf  */
ExprValue val_date(const char *d);     /* expects "YYYY-MM-DD"        */
ExprValue val_logical(int t);          /* t != 0 → .T., else .F.      */

/* ------------------------------------------------------------------ */
/* Lifecycle                                                           */
/* ------------------------------------------------------------------ */

/**
 * free_value — release heap memory held by an ExprValue (only strings).
 * Safe to call on any value, including null-typed ones.
 */
void free_value(ExprValue *v);

/**
 * copy_value — deep-copy an ExprValue. Returns the copy.
 */
ExprValue copy_value(const ExprValue *src);

/* ------------------------------------------------------------------ */
/* Coercion helpers                                                    */
/* ------------------------------------------------------------------ */

/**
 * val_to_double — coerce any value to double.
 * Strings are parsed with atof(); logicals: .T.=1, .F.=0; dates→0.
 */
double val_to_double(const ExprValue *v);

/**
 * val_to_int — coerce any value to int (truncates).
 */
int val_to_int(const ExprValue *v);

/**
 * val_to_string — coerce any value to a human-readable string.
 * Returns a malloc'd string; caller must free it.
 */
char *val_to_string(const ExprValue *v);

/**
 * val_to_logical — coerce to boolean. Non-zero numbers and non-empty strings are .T.
 */
int val_to_logical(const ExprValue *v);

/* ------------------------------------------------------------------ */
/* Comparison                                                          */
/* ------------------------------------------------------------------ */

/**
 * compare_values — compare two values, returns -1 / 0 / +1.
 * Numeric coercion is applied when types differ (number vs number).
 * Strings are compared lexicographically (case-insensitive).
 * Logical: .F. < .T.
 */
int compare_values(const ExprValue *a, const ExprValue *b);

/* ------------------------------------------------------------------ */
/* Formatting                                                          */
/* ------------------------------------------------------------------ */

/**
 * val_type_name — human-readable type name for error messages.
 */
const char *val_type_name(ValType t);

#endif /* _EXPRVALUE_H */
