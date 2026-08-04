/*
 * memfile.h — dBase III .MEM file I/O for SAVE TO / RESTORE FROM
 *
 * Format (32-byte header per variable, followed by payload):
 *   Offset 0x00-0x0A (11 bytes): Variable name, ASCII, NUL-padded
 *   Offset 0x0B (1 byte):        Type byte (0xC3 = character, 0xCE = numeric)
 *   Offset 0x0C-0x0F (4 bytes):  Record ID / serial number (little-endian)
 *   Offset 0x10-0x11 (2 bytes):  Character: payload length | Numeric: width + decimals
 *   Offset 0x12-0x1F (14 bytes): Reserved (usually zero)
 *   Offset 0x20+:                Payload (char: <len> bytes | numeric: 8-byte double LE)
 *
 * File ends with 0x1A (DOS EOF marker).
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
