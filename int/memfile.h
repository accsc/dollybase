/*
 * memfile.h — dBase III .MEM file I/O for SAVE TO / RESTORE FROM
 *
 * Format (per-variable record, 32 bytes fixed + variable-length value):
 *   Offset 0-9:   Variable name (10 bytes, null-padded, uppercase)
 *   Offset 10:    Type (0=C, 1=N, 2=L, 3=D)
 *   Offset 11:    Flag byte (0xce for N/L/D, 0xc3 for C)
 *   Offset 12-13: Value data length (LE, excludes header)
 *   Offset 14-15: File magic (LE, 0xd647)
 *   Offset 16+:   Value data (variable length, null-terminated)
 *
 * File ends with 0x1a (SUB/EOF marker).
 * No file header (MVAR) is written — matches CHKBOOK.MEM format.
 */

#ifndef _MEMFILE_H
#define _MEMFILE_H

/**
 * memfile_save — save selected variables to a .MEM file.
 * names: array of variable names to save (NULL = save all)
 * name_count: number of names (0 if names is NULL = save all)
 * Returns 0 on success, -1 on error.
 */
int memfile_save(const char *path, const char **names, int name_count);

/**
 * memfile_restore — load variables from a .MEM file.
 * additive: if 1, keep existing variables not in file; if 0, clear all first
 * Returns 0 on success, -1 on error.
 */
int memfile_restore(const char *path, int additive);

#endif /* _MEMFILE_H */
