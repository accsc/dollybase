#ifndef _WORKAREA_H
#define _WORKAREA_H

#include <stdio.h>
#include "../libdbase_4/libdbase.h"

#define MAX_WORK_AREAS 10

/* Lifecycle */
void wa_init(void);
void wa_shutdown(void);

/* Work area management */
int  wa_select(int area);          // 1-based area number. Returns 0 on success.
int  wa_get_selected(void);        // 0-based current area index.
DATABASEDBF *wa_db(void);          // Current DBF pointer (NULL if none open).
DATABASEDBF **wa_db_ptr(void);     // Double pointer for skip()/gotos().

/* USE / CLOSE */
int  wa_use(const char *filename, int area);  // area < 0 = next free. Returns 0 on success.
void wa_close(int area);                      // Close specific area (0-based).
void wa_close_all(void);                      // Close all areas.

/* Navigation */
void wa_skip(int n);         // SKIP [n]. Negative n uses gotos.
int  wa_goto(int rec);       // GO <n> (1-based). Returns 0 on success.
void wa_goto_top(void);      // GO TOP
void wa_goto_bottom(void);   // GO BOTTOM

/* Status */
int  wa_recno(void);         // RECNO()
int  wa_reccount(void);      // RECN() / RECCOUNT()
int  wa_eof(void);           // EOF() — 1 if past last record.
int  wa_bof(void);           // BOF() — 1 if at first record.
int  wa_is_deleted(void);    // DELETED() — 1 if current record deleted.

/* CRUD */
int  wa_delete(void);         // DELETE current record.
int  wa_delete_all(void);     // DELETE ALL.
int  wa_recall(void);         // RECALL current record.
int  wa_recall_all(void);     // RECALL ALL.
int  wa_pack(void);           // PACK.
int  wa_zap(void);            // ZAP (= PACK).
int  wa_append_blank(void);   // APPEND BLANK.

/* Field access */
int  wa_field_count(void);
int  wa_field_to_number(const char *name);   // 1-based, 0 if not found.
char *wa_field_name(int idx);                // 1-based. Caller frees.
char wa_field_type(int idx);                 // 1-based. Returns 'C','N','D','L','M', or 0.
char *wa_get_field(int idx);                 // 1-based. Caller frees.
char *wa_get_field_area(int area, int idx);  // 1-based field idx in specific area (0-based). Caller frees.
int  wa_field_to_number_area(int area, const char *name); // 1-based, 0 if not found, in specific area.
char wa_field_type_area(int area, int idx);  // 1-based field idx in specific area.
int  wa_replace(const char *fieldname, const char *value);

/* Utility */
char *wa_dbf_name(void);  // ALIAS(). Caller frees.
int  wa_alias_to_area(const char *alias); // Resolve "A","B",... or DBF name to 0-based area index. Returns -1 if not found.

/* Index support */
int  wa_set_index(const char *index_file);  // SET INDEX TO <file>. Returns 0 on success.
void wa_set_index_clear(void);               // SET INDEX TO (clear).
int  wa_seek(const char *criteria);          // SEEK <expr>. Returns 0 if found.
int  wa_found(void);                         // FOUND() — 1 if last SEEK/LOCATE succeeded.
void wa_set_found(int val);                  // Set FOUND() flag manually.
void wa_index_skip(int n);                   // SKIP using index order.

/* Per-area LOCATE state */
void wa_locate_save(void *for_start, void *for_end, void *while_start, void *db);
void wa_locate_clear(void);
int  wa_locate_active(void);
void *wa_locate_for_start(void);
void *wa_locate_for_end(void);
void *wa_locate_while_start(void);

#endif /* _WORKAREA_H */
