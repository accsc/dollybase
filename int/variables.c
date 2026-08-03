/*
 * variables.c — dollybase variable store implementation
 *
 * Fixed-size array of (name, ExprValue) pairs. Case-insensitive lookup.
 */

#include <string.h>
#include <ctype.h>
#include "variables.h"

static struct {
    char name[256];
    ExprValue value;
} store[MAX_VARS];

static int count = 0;

/* ------------------------------------------------------------------ */
/* Helpers                                                             */
/* ------------------------------------------------------------------ */

static int strcasecmp_portable(const char *a, const char *b)
{
    while (*a && *b) {
        int ca = tolower((unsigned char)*a);
        int cb = tolower((unsigned char)*b);
        if (ca != cb) return ca - cb;
        a++; b++;
    }
    return tolower((unsigned char)*a) - tolower((unsigned char)*b);
}

static int find_index(const char *name)
{
    for (int i = 0; i < count; i++) {
        if (strcasecmp_portable(store[i].name, name) == 0)
            return i;
    }
    return -1;
}

/* ------------------------------------------------------------------ */
/* Public API                                                          */
/* ------------------------------------------------------------------ */

void vars_init(void)
{
    count = 0;
    memset(store, 0, sizeof(store));
}

void vars_shutdown(void)
{
    for (int i = 0; i < count; i++) {
        free_value(&store[i].value);
    }
    count = 0;
}

void vars_set(const char *name, const ExprValue *value)
{
    int idx = find_index(name);
    if (idx >= 0) {
        /* Update existing */
        free_value(&store[idx].value);
        store[idx].value = copy_value(value);
        return;
    }

    /* Insert new */
    if (count >= MAX_VARS) return; /* silently ignore overflow */
    int slot = count++;
    strncpy(store[slot].name, name, sizeof(store[slot].name) - 1);
    store[slot].name[sizeof(store[slot].name) - 1] = '\0';
    store[slot].value = copy_value(value);
}

void vars_set_str(const char *name, const char *str)
{
    ExprValue v = val_string(str ? str : "");
    vars_set(name, &v);
    free_value(&v);
}

ExprValue vars_get(const char *name)
{
    int idx = find_index(name);
    if (idx >= 0) {
        return copy_value(&store[idx].value);
    }
    return val_null();
}

int vars_exists(const char *name)
{
    return find_index(name) >= 0;
}

int vars_count(void)
{
    return count;
}

int vars_get_by_index(int i, const char **out_name, ExprValue *out_value)
{
    if (i < 0 || i >= count)
        return 0;
    *out_name = store[i].name;
    *out_value = copy_value(&store[i].value);
    return 1;
}

int vars_delete(const char *name)
{
    int idx = find_index(name);
    if (idx < 0)
        return 0;
    free_value(&store[idx].value);
    /* Shift remaining elements down */
    for (int j = idx; j < count - 1; j++) {
        store[j] = store[j + 1];
    }
    count--;
    return 1;
}
