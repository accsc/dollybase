/*
 * variables.h — dollybase variable store
 *
 * Simple hash-table-like store: case-insensitive name → ExprValue.
 * Replaces the old memo.c (which stored only raw strings).
 */

#ifndef _VARIABLES_H
#define _VARIABLES_H

#include "exprvalue.h"

/* Maximum number of variables in the store */
#define MAX_VARS 1024

/**
 * vars_init — initialize the variable store. Call once at startup.
 */
void vars_init(void);

/**
 * vars_shutdown — free all stored values and reset the store.
 */
void vars_shutdown(void);

/**
 * vars_set — store or update a variable by name (case-insensitive).
 * The ExprValue is copied (deep copy for strings).
 */
void vars_set(const char *name, const ExprValue *value);
void vars_set_str(const char *name, const char *str);

/**
 * vars_get — retrieve a variable by name. Returns val_null() if not found.
 */
ExprValue vars_get(const char *name);

/**
 * vars_exists — return 1 if the variable exists, 0 otherwise.
 */
int vars_exists(const char *name);

/**
 * vars_count — return the number of variables in the store.
 */
int vars_count(void);

/**
 * vars_get_by_index — retrieve the name and value of the variable at index i.
 * Returns 1 if successful, 0 if i is out of range.
 * The returned name pointer is valid until the next vars mutation.
 * The returned value is a deep copy that the caller must free.
 */
int vars_get_by_index(int i, const char **out_name, ExprValue *out_value);

/**
 * vars_delete — remove a variable by name. Returns 1 if found and removed.
 */
int vars_delete(const char *name);

#endif /* _VARIABLES_H */
