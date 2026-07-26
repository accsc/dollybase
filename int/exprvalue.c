/*
 * exprvalue.c — ExprValue implementation
 */

#include "exprvalue.h"

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <math.h>
#include <ctype.h>
#include <strings.h>

/* ------------------------------------------------------------------ */
/* Constructors                                                        */
/* ------------------------------------------------------------------ */

ExprValue val_null(void) {
    ExprValue v = {0};
    return v;
}

ExprValue val_integer(int i) {
    ExprValue v;
    v.type = VAL_INTEGER;
    v.data.rval = (double)i;
    return v;
}

ExprValue val_real(double d) {
    ExprValue v;
    v.type = VAL_REAL;
    v.data.rval = d;
    return v;
}

ExprValue val_string(const char *s) {
    ExprValue v;
    v.type = VAL_STRING;
    if (s) {
        size_t len = strlen(s);
        v.data.sval = (char *)malloc(len + 1);
        if (v.data.sval) {
            memcpy(v.data.sval, s, len + 1);
        } else {
            v.type = VAL_NULL;
        }
    } else {
        /* Empty string */
        v.data.sval = (char *)malloc(1);
        if (v.data.sval) {
            v.data.sval[0] = '\0';
        } else {
            v.type = VAL_NULL;
        }
    }
    return v;
}

ExprValue val_date(const char *d) {
    ExprValue v;
    v.type = VAL_DATE;
    memset(v.data.dval, 0, sizeof(v.data.dval));
    if (d) {
        strncpy(v.data.dval, d, sizeof(v.data.dval) - 1);
        v.data.dval[sizeof(v.data.dval) - 1] = '\0';
    }
    return v;
}

ExprValue val_logical(int t) {
    ExprValue v;
    v.type = VAL_LOGICAL;
    v.data.rval = (t != 0) ? 1.0 : 0.0;
    return v;
}

/* ------------------------------------------------------------------ */
/* Lifecycle                                                           */
/* ------------------------------------------------------------------ */

void free_value(ExprValue *v) {
    if (!v) return;
    if (v->type == VAL_STRING && v->data.sval) {
        free(v->data.sval);
        v->data.sval = NULL;
    }
}

ExprValue copy_value(const ExprValue *src) {
    ExprValue dst;
    memcpy(&dst, src, sizeof(ExprValue));
    if (dst.type == VAL_STRING && dst.data.sval) {
        size_t len = strlen(dst.data.sval);
        char *copy = (char *)malloc(len + 1);
        if (copy) {
            memcpy(copy, dst.data.sval, len + 1);
            dst.data.sval = copy;
        } else {
            dst.type = VAL_NULL;
        }
    }
    return dst;
}

/* ------------------------------------------------------------------ */
/* Coercion                                                            */
/* ------------------------------------------------------------------ */

double val_to_double(const ExprValue *v) {
    if (!v) return 0.0;
    switch (v->type) {
        case VAL_INTEGER:
        case VAL_REAL:
            return v->data.rval;
        case VAL_STRING:
            return v->data.sval ? atof(v->data.sval) : 0.0;
        case VAL_LOGICAL:
            return v->data.rval;  /* already 0 or 1 */
        case VAL_DATE:
            return 0.0;
        case VAL_NULL:
        default:
            return 0.0;
    }
}

int val_to_int(const ExprValue *v) {
    return (int)val_to_double(v);
}

char *val_to_string(const ExprValue *v) {
    char buf[64];
    if (!v) { char *s = malloc(1); if (s) s[0] = '\0'; return s; }
    switch (v->type) {
        case VAL_INTEGER:
            snprintf(buf, sizeof(buf), "%d", (int)v->data.rval);
            break;
        case VAL_REAL:
            snprintf(buf, sizeof(buf), "%.6g", v->data.rval);
            break;
        case VAL_STRING:
            { const char *src = v->data.sval ? v->data.sval : "";
              char *s = malloc(strlen(src) + 1); if (s) strcpy(s, src); return s; }
        case VAL_DATE:
            { const char *src = v->data.dval;
              char *s = malloc(strlen(src) + 1); if (s) strcpy(s, src); return s; }
        case VAL_LOGICAL:
            { const char *src = v->data.rval != 0.0 ? ".T." : ".F.";
              char *s = malloc(strlen(src) + 1); if (s) strcpy(s, src); return s; }
        case VAL_NULL:
        default:
            buf[0] = '\0';
            break;
    }
    { char *s = malloc(strlen(buf) + 1); if (s) strcpy(s, buf); return s; }
}

int val_to_logical(const ExprValue *v) {
    if (!v) return 0;
    switch (v->type) {
        case VAL_LOGICAL:
            return (v->data.rval != 0.0);
        case VAL_INTEGER:
        case VAL_REAL:
            return (v->data.rval != 0.0);
        case VAL_STRING:
            if (!v->data.sval) return 0;
            /* Empty string is .F., non-empty is .T. */
            return (strlen(v->data.sval) > 0);
        case VAL_DATE:
            {
                const char *d = v->data.dval;
                return (d[0] != '\0' && strcmp(d, "0000-00-00") != 0);
            }
        case VAL_NULL:
        default:
            return 0;
    }
}

/* ------------------------------------------------------------------ */
/* Comparison                                                          */
/* ------------------------------------------------------------------ */

static int strcasecmp_portable(const char *a, const char *b) {
    while (*a && *b) {
        int ca = tolower((unsigned char)*a);
        int cb = tolower((unsigned char)*b);
        if (ca != cb) return ca - cb;
        a++; b++;
    }
    return tolower((unsigned char)*a) - tolower((unsigned char)*b);
}

int compare_values(const ExprValue *a, const ExprValue *b) {
    if (!a || !b) return 0;

    /* Both numeric → compare as doubles */
    if ((a->type == VAL_INTEGER || a->type == VAL_REAL) &&
        (b->type == VAL_INTEGER || b->type == VAL_REAL)) {
        double da = a->data.rval, db = b->data.rval;
        return (da < db) ? -1 : (da > db) ? 1 : 0;
    }

    /* Both strings → case-insensitive lexicographic */
    if (a->type == VAL_STRING && b->type == VAL_STRING) {
        const char *sa = a->data.sval ? a->data.sval : "";
        const char *sb = b->data.sval ? b->data.sval : "";
        return strcasecmp_portable(sa, sb);
    }

    /* Both dates → string compare */
    if (a->type == VAL_DATE && b->type == VAL_DATE) {
        return strcmp(a->data.dval, b->data.dval);
    }

    /* Both logicals */
    if (a->type == VAL_LOGICAL && b->type == VAL_LOGICAL) {
        int la = (int)a->data.rval, lb = (int)b->data.rval;
        return la - lb;
    }

    /* Mixed types: try numeric coercion for number vs string */
    if ((a->type == VAL_STRING || b->type == VAL_STRING) &&
        (a->type != VAL_STRING || b->type != VAL_STRING)) {
        /* If one is a string and the other is numeric, coerce both to double */
        double da = val_to_double(a);
        double db = val_to_double(b);
        return (da < db) ? -1 : (da > db) ? 1 : 0;
    }

    /* Fallback: compare type enum as a last resort */
    if (a->type != b->type) {
        return (int)a->type - (int)b->type;
    }

    return 0;
}

/* ------------------------------------------------------------------ */
/* Formatting                                                          */
/* ------------------------------------------------------------------ */

const char *val_type_name(ValType t) {
    switch (t) {
        case VAL_NULL:    return "NULL";
        case VAL_INTEGER: return "INTEGER";
        case VAL_REAL:    return "REAL";
        case VAL_STRING:  return "STRING";
        case VAL_DATE:    return "DATE";
        case VAL_LOGICAL: return "LOGICAL";
        default:          return "UNKNOWN";
    }
}
