#include "workarea.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>

static DATABASEDBF *areas[MAX_WORK_AREAS];
static int selected = 0;  // 0-based index

/* Per-work-area index state */
typedef struct {
    void *handle;     // NTX or NDX* depending on type
    int type;         // 0 = NDX, 1 = NTX, -1 = none
    int last_found;   // FOUND() state
    int last_page;    // For index-aware skip
    int last_pos;
} IndexState;

static IndexState idx_states[MAX_WORK_AREAS];

/* Per-work-area LOCATE state — so CONTINUE resumes per-area */
typedef struct {
    void *for_start;       /* Token* of FOR expr start   */
    void *for_end;         /* Token* of FOR expr end     */
    void *while_start;     /* Token* of WHILE expr start */
    void *db;              /* DATABASEDBF* LOCATE ran on */
    int active;            /* 1 if a LOCATE was executed */
} LocateState;

static LocateState locate_states[MAX_WORK_AREAS];

/* Per-work-area custom alias (set via USE ... ALIAS name) */
static char *custom_aliases[MAX_WORK_AREAS];

/* ------------------------------------------------------------------ */
/* Lifecycle                                                           */
/* ------------------------------------------------------------------ */

void wa_init(void)
{
    memset(areas, 0, sizeof(areas));
    memset(idx_states, 0, sizeof(idx_states));
    for (int i = 0; i < MAX_WORK_AREAS; i++) {
        idx_states[i].type = -1;
        custom_aliases[i] = NULL;
    }
    selected = 0;
}

void wa_shutdown(void)
{
    for (int i = 0; i < MAX_WORK_AREAS; i++) {
        if (areas[i]) {
            free(areas[i]);
            areas[i] = NULL;
        }
        free(custom_aliases[i]);
        custom_aliases[i] = NULL;
    }
    selected = 0;
}

/* ------------------------------------------------------------------ */
/* Work area management                                                */
/* ------------------------------------------------------------------ */

int wa_select(int area)
{
    int idx = area - 1;
    if (idx < 0 || idx >= MAX_WORK_AREAS) {
        fprintf(stderr, "prg: SELECT area must be 1-%d\n", MAX_WORK_AREAS);
        return -1;
    }
    selected = idx;
    return 0;
}

int wa_get_selected(void)
{
    return selected;
}

DATABASEDBF *wa_db(void)
{
    return areas[selected];
}

DATABASEDBF **wa_db_ptr(void)
{
    return &areas[selected];
}

/* ------------------------------------------------------------------ */
/* USE / CLOSE                                                         */
/* ------------------------------------------------------------------ */

int wa_use(const char *filename, int area, const char *alias)
{
    char path[1024];
    if (strchr(filename, '.') == NULL) {
        snprintf(path, sizeof(path), "%s.dbf", filename);
    } else {
        strncpy(path, filename, sizeof(path) - 1);
        path[sizeof(path) - 1] = '\0';
    }

    int idx = area;
    if (idx < 0) {
        idx = 0;
        while (idx < MAX_WORK_AREAS && areas[idx] != NULL)
            idx++;
        if (idx >= MAX_WORK_AREAS) {
            fprintf(stderr, "prg: no free work area\n");
            return -1;
        }
    } else {
        idx = idx - 1;
        if (idx < 0 || idx >= MAX_WORK_AREAS) {
            fprintf(stderr, "prg: invalid work area %d\n", idx + 1);
            return -1;
        }
        if (areas[idx]) {
            free(areas[idx]);
            areas[idx] = NULL;
        }
        free(custom_aliases[idx]);
        custom_aliases[idx] = NULL;
    }

    DATABASEDBF *db = calloc(1, sizeof(DATABASEDBF));
    if (!db) {
        fprintf(stderr, "prg: out of memory\n");
        return -1;
    }
    use((char *)path, &db);
    // use() does memcpy(*asp, internal, sizeof) on success.
    // On failure (file not found), use() frees its internal copy and returns
    // without touching *asp, leaving our malloc'd block untouched.
    // We detect failure by checking if name[0] is still '\0'.
    if (db->name[0] == '\0') {
        fprintf(stderr, "prg: cannot open '%s'\n", path);
        free(db);
        return -1;
    }
    areas[idx] = db;
    selected = idx;

    /* Set custom alias */
    if (alias && *alias) {
        custom_aliases[idx] = strdup(alias);
    } else {
        custom_aliases[idx] = NULL;
    }

    return 0;
}

void wa_close(int area)
{
    if (area >= 0 && area < MAX_WORK_AREAS) {
        if (areas[area]) {
            free(areas[area]);
            areas[area] = NULL;
        }
        free(custom_aliases[area]);
        custom_aliases[area] = NULL;
        if (area == selected)
            selected = 0;
    }
}

void wa_close_all(void)
{
    for (int i = 0; i < MAX_WORK_AREAS; i++) {
        if (areas[i]) {
            free(areas[i]);
            areas[i] = NULL;
        }
        free(custom_aliases[i]);
        custom_aliases[i] = NULL;
    }
    selected = 0;
}

/* ------------------------------------------------------------------ */
/* Navigation                                                          */
/* ------------------------------------------------------------------ */

void wa_skip(int n)
{
    DATABASEDBF *db = wa_db();
    if (!db) return;

    if (n > 0) {
        for (int i = 0; i < n; i++)
            skip(wa_db_ptr());
    } else if (n < 0) {
        int target = db->current + n;
        if (target < 1) target = 1;
        gotos(wa_db_ptr(), target);
    }
}

int wa_goto(int rec)
{
    DATABASEDBF *db = wa_db();
    if (!db) return -1;
    return gotos(wa_db_ptr(), rec);
}

void wa_goto_top(void)
{
    DATABASEDBF *db = wa_db();
    if (!db) return;
    gotos(wa_db_ptr(), 1);
}

void wa_goto_bottom(void)
{
    DATABASEDBF *db = wa_db();
    if (!db) return;
    gotos(wa_db_ptr(), reccount(db));
}

/* ------------------------------------------------------------------ */
/* Status                                                              */
/* ------------------------------------------------------------------ */

int wa_recno(void)
{
    DATABASEDBF *db = wa_db();
    if (!db) return 0;
    return recno(db);
}

int wa_reccount(void)
{
    DATABASEDBF *db = wa_db();
    if (!db) return 0;
    return reccount(db);
}

int wa_eof(void)
{
    DATABASEDBF *db = wa_db();
    if (!db) return 1;
    return (eof_dbf(db) == VERITAS);
}

int wa_bof(void)
{
    DATABASEDBF *db = wa_db();
    if (!db) return 1;
    return (bof(db) == VERITAS);
}

int wa_is_deleted(void)
{
    DATABASEDBF *db = wa_db();
    if (!db) return 0;
    return (is_deleted(db) == VERITAS);
}

/* ------------------------------------------------------------------ */
/* CRUD                                                                */
/* ------------------------------------------------------------------ */

int wa_delete(void)
{
    DATABASEDBF *db = wa_db();
    if (!db) return -1;
    return delete(db);
}

int wa_delete_all(void)
{
    DATABASEDBF *db = wa_db();
    if (!db) return -1;
    return delete_all(db);
}

int wa_recall(void)
{
    DATABASEDBF *db = wa_db();
    if (!db) return -1;
    return recall(db);
}

int wa_recall_all(void)
{
    DATABASEDBF *db = wa_db();
    if (!db) return -1;
    return recall_all(db);
}

int wa_pack(void)
{
    DATABASEDBF *db = wa_db();
    if (!db) return -1;

    char db_name[1024];
    strncpy(db_name, db->name, sizeof(db_name) - 1);
    db_name[sizeof(db_name) - 1] = '\0';

    int saved_rec = db->current;
    int sel = wa_get_selected();

    /* If database has a DBT file (tipo 3 = DBT memo, tipo 4 = DBT memo),
     * use the DBT-aware pack function */
    if (db->tipo == 3 || db->tipo == 4) {
        char dbt_name[1024];
        get_dbt(db->name, dbt_name);
        pack_db_with_dbt_file(db, dbt_name);
    } else {
        pack(db);
    }

    /* Close and re-open to reload header (recnos changed on disk) */
    wa_close(sel);
    wa_use(db_name, sel, custom_aliases[sel]);
    wa_goto(saved_rec);
    if (wa_eof())
        wa_goto_bottom();

    return 0;
}

int wa_zap(void)
{
    return wa_pack();
}

int wa_append_blank(void)
{
    return append_blank(wa_db_ptr());
}

/* ------------------------------------------------------------------ */
/* Field access                                                        */
/* ------------------------------------------------------------------ */

int wa_field_count(void)
{
    DATABASEDBF *db = wa_db();
    if (!db) return 0;
    return fields_num(db);
}

int wa_field_to_number(const char *name)
{
    DATABASEDBF *db = wa_db();
    if (!db) return 0;
    int result = field_to_number(db, (char *)name);
    if (result < 0) return 0;
    return result;
}

char *wa_field_name(int idx)
{
    DATABASEDBF *db = wa_db();
    if (!db) return NULL;
    char buf[257];
    char *p = buf;
    if (field_name(db, idx, &p) != 0)
        return NULL;
    return strdup(buf);
}

char wa_field_type(int idx)
{
    DATABASEDBF *db = wa_db();
    if (!db) return 0;
    int t = dfield_type(db, idx);
    return (char)t;
}

char *wa_get_field(int idx)
{
    DATABASEDBF *db = wa_db();
    if (!db) return NULL;
    char *buf = malloc(1024);
    if (!buf) return NULL;
    char *p = buf;
    get_field(db, idx, &p);
    char *result = strdup(buf);
    free(buf);
    return result;
}

int wa_field_to_number_area(int area, const char *name)
{
    if (area < 0 || area >= MAX_WORK_AREAS || !areas[area]) return 0;
    int result = field_to_number(areas[area], (char *)name);
    if (result < 0) return 0;
    return result;
}

char wa_field_type_area(int area, int idx)
{
    if (area < 0 || area >= MAX_WORK_AREAS || !areas[area]) return 0;
    return (char)dfield_type(areas[area], idx);
}

char *wa_get_field_area(int area, int idx)
{
    if (area < 0 || area >= MAX_WORK_AREAS || !areas[area]) return NULL;
    char *buf = malloc(1024);
    if (!buf) return NULL;
    char *p = buf;
    get_field(areas[area], idx, &p);
    char *result = strdup(buf);
    free(buf);
    return result;
}

int wa_replace(const char *fieldname, const char *value)
{
    DATABASEDBF *db = wa_db();
    if (!db) return -1;
    return replace(db, (char *)fieldname, (char *)value);
}

/* ------------------------------------------------------------------ */
/* Utility                                                             */
/* ------------------------------------------------------------------ */

char *wa_dbf_name(void)
{
    int idx = selected;
    /* If custom alias is set, return it */
    if (custom_aliases[idx])
        return strdup(custom_aliases[idx]);

    DATABASEDBF *db = wa_db();
    if (!db) return strdup("");
    char *name = NULL;
    DBF(db, &name);
    if (name) {
        /* Strip .dbf extension for ALIAS() — dBASE returns name without extension */
        char *dot = strrchr(name, '.');
        if (dot)
            *dot = '\0';
        return name;
    }
    return strdup("");
}

/* Resolve alias to 0-based area index.
   Accepts single-letter aliases ("A"=area 0, "B"=area 1, ...)
   or the DBF name (case-insensitive match on the name field). */
int wa_alias_to_area(const char *alias)
{
    if (!alias || !*alias) return -1;

    /* Single-letter alias: A=0, B=1, ... J=9 */
    if (alias[1] == '\0') {
        int c = toupper((unsigned char)alias[0]);
        if (c >= 'A' && c < 'A' + MAX_WORK_AREAS) {
            int idx = c - 'A';
            if (areas[idx])
                return idx;
        }
    }

    /* Try matching against custom aliases first (case-insensitive) */
    for (int i = 0; i < MAX_WORK_AREAS; i++) {
        if (areas[i] && custom_aliases[i] && strcasecmp(custom_aliases[i], alias) == 0)
            return i;
    }

    /* Try matching against DBF names (case-insensitive) */
    for (int i = 0; i < MAX_WORK_AREAS; i++) {
        if (areas[i] && strcasecmp(areas[i]->name, alias) == 0)
            return i;
    }

    return -1;
}

/* ------------------------------------------------------------------ */
/* Index support                                                       */
/* ------------------------------------------------------------------ */

static void idx_close(void)
{
    IndexState *st = &idx_states[selected];
    if (st->type == 1 && st->handle) {
        /* NTX is returned by value, stored in heap */
        free(st->handle);
    } else if (st->type == 0 && st->handle) {
        /* NDX is allocated via calloc in use_ndx */
        free(st->handle);
    }
    st->handle = NULL;
    st->type = -1;
    st->last_found = 0;
}

int wa_set_index(const char *index_file)
{
    DATABASEDBF *db = wa_db();
    if (!db) return -1;

    idx_close();

    char path[1024];
    if (strchr(index_file, '.') == NULL) {
        // Try .ntx first, then .ndx
        snprintf(path, sizeof(path), "%s.ntx", index_file);
    } else {
        strncpy(path, index_file, sizeof(path) - 1);
        path[sizeof(path) - 1] = '\0';
    }

    IndexState *st = &idx_states[selected];
    char *ext = strrchr(path, '.');

    if (ext && strcasecmp(ext, ".ntx") == 0) {
        NTX *ntx = malloc(sizeof(NTX));
        if (ntx) {
            *ntx = use_ntx(path);
            if (ntx->type == 0) {
                /* use_ntx returns type=0 on failure */
                free(ntx);
                return -1;
            }
            st->handle = ntx;
            st->type = 1;
            return 0;
        }
        return -1;
    }

    if (ext && strcasecmp(ext, ".ndx") == 0) {
        char *path_copy = strdup(path);
        NDX *ndx = use_ndx(path_copy);
        if (ndx) {
            st->handle = ndx;
            st->type = 0;
            return 0;
        }
        free(path_copy);
        return -1;
    }

    // No extension — try .ntx then .ndx
    snprintf(path, sizeof(path), "%s.ntx", index_file);
    NTX *ntx = malloc(sizeof(NTX));
    if (ntx) {
        *ntx = use_ntx(path);
        if (ntx->type != 0) {
            st->handle = ntx;
            st->type = 1;
            return 0;
        }
        free(ntx);
    }

    snprintf(path, sizeof(path), "%s.ndx", index_file);
    {
        char *path_copy = strdup(path);
        NDX *ndx = use_ndx(path_copy);
        if (ndx) {
            st->handle = ndx;
            st->type = 0;
            return 0;
        }
        free(path_copy);
    }

    return -1;
}

void wa_set_index_clear(void)
{
    idx_close();
}

int wa_seek(const char *criteria)
{
    IndexState *st = &idx_states[selected];
    DATABASEDBF *db = wa_db();
    if (!db || !st->handle || st->type < 0) {
        st->last_found = 0;
        return -1;
    }

    FOUND fin;
    if (st->type == 0) {
        fin = seek_ndx_btree((NDX *)st->handle, (char *)criteria);
    } else {
        fin = seek_ntx_btree((NTX *)st->handle, (char *)criteria);
    }

    if (fin.recno > 0 && fin.recno <= db->recnos) {
        gotos(wa_db_ptr(), fin.recno);
        st->last_found = 1;
        st->last_page = fin.page;
        st->last_pos = fin.pos;
        return 0;
    }

    st->last_found = 0;
    return -1;
}

int wa_found(void)
{
    IndexState *st = &idx_states[selected];
    return st->last_found;
}

void wa_set_found(int val)
{
    idx_states[selected].last_found = val;
}

void wa_index_skip(int n)
{
    IndexState *st = &idx_states[selected];
    DATABASEDBF *db = wa_db();
    if (!db || !st->handle || st->type < 0) {
        wa_skip(n);
        return;
    }

    if (n > 0) {
        for (int i = 0; i < n; i++) {
            FOUND fin;
            if (st->type == 0) {
                fin = search_ndx_next((NTX *)st->handle, NULL,
                    st->last_page, st->last_pos);
            } else {
                fin = search_ntx_next((NTX *)st->handle, NULL,
                    st->last_page, st->last_pos);
            }
            if (fin.recno > 0 && fin.recno <= db->recnos) {
                gotos(wa_db_ptr(), fin.recno);
                st->last_page = fin.page;
                st->last_pos = fin.pos;
            } else {
                break;
            }
        }
    }
}

/* ------------------------------------------------------------------ */
/* Per-area LOCATE state                                               */
/* ------------------------------------------------------------------ */

void wa_locate_save(void *for_start, void *for_end, void *while_start, void *db)
{
    LocateState *ls = &locate_states[selected];
    ls->for_start = for_start;
    ls->for_end = for_end;
    ls->while_start = while_start;
    ls->db = db;
    ls->active = 1;
}

void wa_locate_clear(void)
{
    locate_states[selected].active = 0;
}

int wa_locate_active(void)
{
    LocateState *ls = &locate_states[selected];
    if (!ls->active || ls->db != wa_db()) {
        ls->active = 0;
        return 0;
    }
    return 1;
}

void *wa_locate_for_start(void)
{
    return locate_states[selected].for_start;
}

void *wa_locate_for_end(void)
{
    return locate_states[selected].for_end;
}

void *wa_locate_while_start(void)
{
    return locate_states[selected].while_start;
}
