#include "workarea.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <time.h>
#include <ctype.h>

/* ------------------------------------------------------------------ */
/* CP437 <-> UTF-8 encoding conversion                                */
/* DBF files from dBASE store text in CP437 (IBM PC code page).       */
/* ncurses / terminals expect UTF-8.                                  */
/* The lookup table maps CP437 bytes 0x80-0xFF to their Unicode code  */
/* points. Bytes 0x00-0x7F are ASCII and pass through unchanged.      */
/* ------------------------------------------------------------------ */

static unsigned int cp437_to_unicode[256]; /* populated by cp437_init() */

/* Initialize CP437 table — called once at startup */
static void cp437_init(void)
{
    /* 0x00-0x7F are already ASCII (identity) */
    /* 0x80-0x8F: accented lowercase */
    cp437_to_unicode[0x80]=199; cp437_to_unicode[0x81]=252; cp437_to_unicode[0x82]=233; cp437_to_unicode[0x83]=226;
    cp437_to_unicode[0x84]=228; cp437_to_unicode[0x85]=224; cp437_to_unicode[0x86]=229; cp437_to_unicode[0x87]=231;
    cp437_to_unicode[0x88]=234; cp437_to_unicode[0x89]=235; cp437_to_unicode[0x8A]=232; cp437_to_unicode[0x8B]=239;
    cp437_to_unicode[0x8C]=238; cp437_to_unicode[0x8D]=236; cp437_to_unicode[0x8E]=196; cp437_to_unicode[0x8F]=197;
    /* 0x90-0x9F: accented uppercase, symbols */
    cp437_to_unicode[0x90]=201; cp437_to_unicode[0x91]=230; cp437_to_unicode[0x92]=198; cp437_to_unicode[0x93]=244;
    cp437_to_unicode[0x94]=246; cp437_to_unicode[0x95]=242; cp437_to_unicode[0x96]=251; cp437_to_unicode[0x97]=249;
    cp437_to_unicode[0x98]=255; cp437_to_unicode[0x99]=214; cp437_to_unicode[0x9A]=220; cp437_to_unicode[0x9B]=162;
    cp437_to_unicode[0x9C]=163; cp437_to_unicode[0x9D]=165; cp437_to_unicode[0x9E]=8359; cp437_to_unicode[0x9F]=402;
    /* 0xA0-0xAF: accented, punctuation */
    cp437_to_unicode[0xA0]=225; cp437_to_unicode[0xA1]=237; cp437_to_unicode[0xA2]=243; cp437_to_unicode[0xA3]=250;
    cp437_to_unicode[0xA4]=241; cp437_to_unicode[0xA5]=209; cp437_to_unicode[0xA6]=170; cp437_to_unicode[0xA7]=186;
    cp437_to_unicode[0xA8]=191; cp437_to_unicode[0xA9]=8976; cp437_to_unicode[0xAA]=172; cp437_to_unicode[0xAB]=189;
    cp437_to_unicode[0xAC]=188; cp437_to_unicode[0xAD]=161; cp437_to_unicode[0xAE]=171; cp437_to_unicode[0xAF]=187;
    /* 0xB0-0xBF: box drawing */
    cp437_to_unicode[0xB0]=9617;cp437_to_unicode[0xB1]=9618;cp437_to_unicode[0xB2]=9619;cp437_to_unicode[0xB3]=9474;
    cp437_to_unicode[0xB4]=9508;cp437_to_unicode[0xB5]=9569;cp437_to_unicode[0xB6]=9570;cp437_to_unicode[0xB7]=9558;
    cp437_to_unicode[0xB8]=9557;cp437_to_unicode[0xB9]=9571;cp437_to_unicode[0xBA]=9553;cp437_to_unicode[0xBB]=9559;
    cp437_to_unicode[0xBC]=9565;cp437_to_unicode[0xBD]=9564;cp437_to_unicode[0xBE]=9563;cp437_to_unicode[0xBF]=9488;
    /* 0xC0-0xCF: box drawing */
    cp437_to_unicode[0xC0]=9492;cp437_to_unicode[0xC1]=9524;cp437_to_unicode[0xC2]=9516;cp437_to_unicode[0xC3]=9500;
    cp437_to_unicode[0xC4]=9472;cp437_to_unicode[0xC5]=9532;cp437_to_unicode[0xC6]=9566;cp437_to_unicode[0xC7]=9567;
    cp437_to_unicode[0xC8]=9562;cp437_to_unicode[0xC9]=9556;cp437_to_unicode[0xCA]=9577;cp437_to_unicode[0xCB]=9574;
    cp437_to_unicode[0xCC]=9568;cp437_to_unicode[0xCD]=9552;cp437_to_unicode[0xCE]=9580;cp437_to_unicode[0xCF]=9575;
    /* 0xD0-0xDF: box drawing, blocks */
    cp437_to_unicode[0xD0]=9576;cp437_to_unicode[0xD1]=9572;cp437_to_unicode[0xD2]=9573;cp437_to_unicode[0xD3]=9561;
    cp437_to_unicode[0xD4]=9560;cp437_to_unicode[0xD5]=9554;cp437_to_unicode[0xD6]=9555;cp437_to_unicode[0xD7]=9579;
    cp437_to_unicode[0xD8]=9578;cp437_to_unicode[0xD9]=9496;cp437_to_unicode[0xDA]=9484;cp437_to_unicode[0xDB]=9608;
    cp437_to_unicode[0xDC]=9604;cp437_to_unicode[0xDD]=9612;cp437_to_unicode[0xDE]=9616;cp437_to_unicode[0xDF]=9600;
    /* 0xE0-0xEF: Greek, math */
    cp437_to_unicode[0xE0]=945; cp437_to_unicode[0xE1]=223; cp437_to_unicode[0xE2]=915; cp437_to_unicode[0xE3]=960;
    cp437_to_unicode[0xE4]=931; cp437_to_unicode[0xE5]=963; cp437_to_unicode[0xE6]=181; cp437_to_unicode[0xE7]=964;
    cp437_to_unicode[0xE8]=934; cp437_to_unicode[0xE9]=920; cp437_to_unicode[0xEA]=937; cp437_to_unicode[0xEB]=948;
    cp437_to_unicode[0xEC]=8734;cp437_to_unicode[0xED]=966; cp437_to_unicode[0xEE]=949; cp437_to_unicode[0xEF]=8745;
    /* 0xF0-0xFF: math, misc */
    cp437_to_unicode[0xF0]=8801;cp437_to_unicode[0xF1]=177; cp437_to_unicode[0xF2]=8805;cp437_to_unicode[0xF3]=8804;
    cp437_to_unicode[0xF4]=8992;cp437_to_unicode[0xF5]=8993;cp437_to_unicode[0xF6]=247; cp437_to_unicode[0xF7]=8776;
    cp437_to_unicode[0xF8]=176; cp437_to_unicode[0xF9]=8729;cp437_to_unicode[0xFA]=183; cp437_to_unicode[0xFB]=8730;
    cp437_to_unicode[0xFC]=8319;cp437_to_unicode[0xFD]=178; cp437_to_unicode[0xFE]=9632;cp437_to_unicode[0xFF]=160;
}

/* Write a Unicode code point as UTF-8. Returns number of bytes written. */
static int utf8_encode(char *buf, unsigned int cp)
{
    if (cp < 0x80) {
        buf[0] = (char)cp;
        return 1;
    } else if (cp < 0x800) {
        buf[0] = (char)(0xC0 | (cp >> 6));
        buf[1] = (char)(0x80 | (cp & 0x3F));
        return 2;
    } else {
        buf[0] = (char)(0xE0 | (cp >> 12));
        buf[1] = (char)(0x80 | ((cp >> 6) & 0x3F));
        buf[2] = (char)(0x80 | (cp & 0x3F));
        return 3;
    }
}

static char *cp437_to_utf8(const char *src)
{
    if (!src) return NULL;
    size_t len = strlen(src);
    char *out = malloc(len * 3 + 1);
    if (!out) return NULL;
    size_t oi = 0;
    for (size_t i = 0; i < len; i++) {
        unsigned char c = (unsigned char)src[i];
        unsigned int cp = (c < 0x80) ? c : cp437_to_unicode[c];
        oi += utf8_encode(out + oi, cp);
    }
    out[oi] = '\0';
    return out;
}

/* Reverse lookup: UTF-8 -> CP437 byte (best effort) */
static unsigned int unicode_to_cp437[1200];
static int cp437_rev_init_done = 0;

static void cp437_rev_init(void)
{
    if (cp437_rev_init_done) return;
    memset(unicode_to_cp437, 0, sizeof(unicode_to_cp437));
    for (int i = 0; i < 256; i++) {
        unsigned int cp = cp437_to_unicode[(unsigned char)i];
        if (cp < 1200)
            unicode_to_cp437[cp] = (unsigned char)i;
    }
    cp437_rev_init_done = 1;
}

static char *utf8_to_cp437(const char *src)
{
    if (!src) return NULL;
    cp437_rev_init();
    size_t len = strlen(src);
    char *out = malloc(len + 1);
    if (!out) return NULL;
    size_t oi = 0;
    for (size_t i = 0; i < len; i++) {
        unsigned char c = (unsigned char)src[i];
        if (c < 0x80) {
            out[oi++] = (char)c;
        } else if ((c & 0xE0) == 0xC0 && i + 1 < len) {
            unsigned char c2 = (unsigned char)src[i + 1];
            unsigned int cp = ((c & 0x1F) << 6) | (c2 & 0x3F);
            if (cp < 1200 && unicode_to_cp437[cp]) {
                out[oi++] = (char)unicode_to_cp437[cp];
            }
            /* else drop */
            i++;
        } else if ((c & 0xF0) == 0xE0 && i + 2 < len) {
            unsigned char c2 = (unsigned char)src[i + 1];
            unsigned char c3 = (unsigned char)src[i + 2];
            unsigned int cp = ((c & 0x0F) << 12) | ((c2 & 0x3F) << 6) | (c3 & 0x3F);
            if (cp < 1200 && unicode_to_cp437[cp]) {
                out[oi++] = (char)unicode_to_cp437[cp];
            }
            i += 2;
        }
    }
    out[oi] = '\0';
    return out;
}

static DATABASEDBF *areas[MAX_WORK_AREAS];
static int selected = 0;  // 0-based index

/* Per-work-area index state */
#define MAX_INDEX_TAGS 10

typedef struct {
    void *handle;     // NTX or NDX* depending on type
    int type;         // 0 = NDX, 1 = NTX, -1 = none
    int last_page;    // For index-aware skip
    int last_pos;
} IndexTag;

typedef struct {
    IndexTag tags[MAX_INDEX_TAGS];
    int tag_count;    // Number of loaded tags
    int active_order; // 0 = no index, 1..tag_count = active tag
    int last_found;   // FOUND() state
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
    cp437_init();
    memset(areas, 0, sizeof(areas));
    memset(idx_states, 0, sizeof(idx_states));
    for (int i = 0; i < MAX_WORK_AREAS; i++) {
        idx_states[i].active_order = 0;
        for (int j = 0; j < MAX_INDEX_TAGS; j++)
            idx_states[i].tags[j].type = -1;
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
    if (db->name[0] == '\0' || db->tipo == 0) {
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

/* Update the DBF header's last-update date to today (MMDDYYYY format) */
static void update_dbf_date(DATABASEDBF *db)
{
    if (!db) return;
    time_t now = time(NULL);
    struct tm *t = localtime(&now);
    unsigned char date_bytes[3];
    date_bytes[0] = (unsigned char)(t->tm_year % 100);  /* YY - years since 1900 */
    date_bytes[1] = (unsigned char)(t->tm_mon + 1);      /* MM */
    date_bytes[2] = (unsigned char)(t->tm_mday);          /* DD */
    snprintf(db->date, sizeof(db->date), "%02d/%02d/20%02d",
             t->tm_mon + 1, t->tm_mday, t->tm_year % 100);
    /* Write 3-byte date at header offsets 1-3 (YY, MM, DD) */
    FILE *f = fopen(db->name, "r+b");
    if (f) {
        fseek(f, 1, SEEK_SET);
        fwrite(date_bytes, 1, 3, f);
        fclose(f);
    }
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

    /* Update last-update date before re-opening */
    update_dbf_date(db);

    /* Close and re-open to reload header (recnos changed on disk) */
    wa_close(sel);
    wa_use(db_name, sel + 1, custom_aliases[sel]);
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
    DATABASEDBF *db = wa_db();
    int rc = append_blank(wa_db_ptr());
    if (rc == 0 && db)
        update_dbf_date(db);
    return rc;
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
    char *utf8 = cp437_to_utf8(buf);
    free(buf);
    return utf8;
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
    char *utf8 = cp437_to_utf8(buf);
    free(buf);
    return utf8;
}

int wa_replace(const char *fieldname, const char *value)
{
    DATABASEDBF *db = wa_db();
    if (!db) return -1;

    /* DBF date fields store YYYYMMDD (8 bytes, no dashes).
       Our internal format is YYYY-MM-DD (10 chars). Strip dashes. */
    int fidx = field_to_number(db, (char *)fieldname);
    char ftype = dfield_type(db, fidx);
    char *write_val = (char *)value;
    char *stripped = NULL;

    if (ftype == 'D' && strlen(value) == 10 && value[4] == '-' && value[7] == '-') {
        /* Convert YYYY-MM-DD -> YYYYMMDD */
        stripped = malloc(9);
        if (stripped) {
            sprintf(stripped, "%c%c%c%c%c%c%c%c",
                    value[0], value[1], value[2], value[3],
                    value[5], value[6], value[8], value[9]);
            write_val = stripped;
        }
    }

    char *latin1 = utf8_to_cp437(write_val);
    int rc = replace(db, (char *)fieldname, latin1 ? latin1 : write_val);
    free(latin1);
    free(stripped);
    if (rc == 0)
        update_dbf_date(db);
    return rc;
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
    for (int i = 0; i < st->tag_count; i++) {
        IndexTag *tag = &st->tags[i];
        if (tag->type == 1 && tag->handle) {
            free(tag->handle);
        } else if (tag->type == 0 && tag->handle) {
            free(tag->handle);
        }
        tag->handle = NULL;
        tag->type = -1;
        tag->last_page = 0;
        tag->last_pos = 0;
    }
    st->tag_count = 0;
    st->active_order = 0;
    st->last_found = 0;
}

/* Helper: close a single tag at a specific index */
static void idx_close_tag(IndexState *st, int idx)
{
    IndexTag *tag = &st->tags[idx];
    if (tag->type == 1 && tag->handle) {
        free(tag->handle);
    } else if (tag->type == 0 && tag->handle) {
        free(tag->handle);
    }
    tag->handle = NULL;
    tag->type = -1;
    tag->last_page = 0;
    tag->last_pos = 0;
}

/* Helper: get the active tag pointer, or NULL if order is 0 or invalid */
static IndexTag *idx_active_tag(IndexState *st)
{
    if (st->active_order < 1 || st->active_order > st->tag_count)
        return NULL;
    return &st->tags[st->active_order - 1];
}

/* Internal: load a single index file into the next available tag slot.
   Returns 0 on success, -1 on failure.  Sets active_order to the new tag. */
static int idx_load_tag(const char *index_file)
{
    DATABASEDBF *db = wa_db();
    if (!db) return -1;

    IndexState *st = &idx_states[selected];
    if (st->tag_count >= MAX_INDEX_TAGS)
        return -1;

    char path[1024];
    if (strchr(index_file, '.') == NULL) {
        snprintf(path, sizeof(path), "%s.ntx", index_file);
    } else {
        strncpy(path, index_file, sizeof(path) - 1);
        path[sizeof(path) - 1] = '\0';
    }

    int tag_idx = st->tag_count;
    IndexTag *tag = &st->tags[tag_idx];
    char *ext = strrchr(path, '.');

    if (ext && strcasecmp(ext, ".ntx") == 0) {
        NTX *ntx = malloc(sizeof(NTX));
        if (ntx) {
            *ntx = use_ntx(path);
            if (ntx->type == 0) {
                free(ntx);
                return -1;
            }
            tag->handle = ntx;
            tag->type = 1;
            st->tag_count++;
            st->active_order = tag_idx + 1;
            return 0;
        }
        return -1;
    }

    if (ext && strcasecmp(ext, ".ndx") == 0) {
        char *path_copy = strdup(path);
        NDX *ndx = use_ndx(path_copy);
        if (ndx) {
            tag->handle = ndx;
            tag->type = 0;
            st->tag_count++;
            st->active_order = tag_idx + 1;
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
            tag->handle = ntx;
            tag->type = 1;
            st->tag_count++;
            st->active_order = tag_idx + 1;
            return 0;
        }
        free(ntx);
    }

    snprintf(path, sizeof(path), "%s.ndx", index_file);
    {
        char *path_copy = strdup(path);
        NDX *ndx = use_ndx(path_copy);
        if (ndx) {
            tag->handle = ndx;
            tag->type = 0;
            st->tag_count++;
            st->active_order = tag_idx + 1;
            return 0;
        }
        free(path_copy);
    }

    return -1;
}

int wa_set_index(const char *index_file)
{
    idx_close();  /* Clear all existing tags first */
    return idx_load_tag(index_file);
}

void wa_set_index_clear(void)
{
    idx_close();
}

void wa_set_order(int order)
{
    IndexState *st = &idx_states[selected];
    if (order == 0) {
        /* Suspend index use */
        st->active_order = 0;
    } else if (order >= 1 && order <= st->tag_count) {
        st->active_order = order;
    }
    /* Silently ignore out-of-range values */
}

int wa_seek(const char *criteria)
{
    IndexState *st = &idx_states[selected];
    IndexTag *tag = idx_active_tag(st);
    DATABASEDBF *db = wa_db();
    if (!db || !tag || !tag->handle || tag->type < 0) {
        st->last_found = 0;
        return -1;
    }

    FOUND fin;
    if (tag->type == 0) {
        fin = seek_ndx_btree((NDX *)tag->handle, (char *)criteria);
    } else {
        fin = seek_ntx_btree((NTX *)tag->handle, (char *)criteria);
    }

    if (fin.recno > 0 && fin.recno <= db->recnos) {
        gotos(wa_db_ptr(), fin.recno);
        st->last_found = 1;
        tag->last_page = fin.page;
        tag->last_pos = fin.pos;
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
    IndexTag *tag = idx_active_tag(st);
    DATABASEDBF *db = wa_db();
    if (!db || !tag || !tag->handle || tag->type < 0) {
        wa_skip(n);
        return;
    }

    if (n > 0) {
        for (int i = 0; i < n; i++) {
            FOUND fin;
            if (tag->type == 0) {
                fin = search_ndx_next((NTX *)tag->handle, NULL,
                    tag->last_page, tag->last_pos);
            } else {
                fin = search_ntx_next((NTX *)tag->handle, NULL,
                    tag->last_page, tag->last_pos);
            }
            if (fin.recno > 0 && fin.recno <= db->recnos) {
                gotos(wa_db_ptr(), fin.recno);
                tag->last_page = fin.page;
                tag->last_pos = fin.pos;
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
