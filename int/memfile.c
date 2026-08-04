/*
 * memfile.c — dBase III .MEM file I/O for SAVE TO / RESTORE FROM
 *
 * Format (32-byte header per variable, followed by payload):
 *   Offset 0x00-0x0A (11 bytes): Variable name, ASCII, NUL-padded
 *   Offset 0x0B (1 byte):        Type byte:
 *                                 0xC3 = character, 0xC4 = date,
 *                                 0xCC = logical, 0xCE = numeric
 *   Offset 0x0C-0x0F (4 bytes):  Record ID / serial number (little-endian)
 *                                 FoxBase+ uses 0x66CA in upper bytes,
 *                                 dBase III uses 0x47D6 in upper bytes
 *   Offset 0x10-0x11 (2 bytes):  Meaning depends on type:
 *                                 Character: payload length (uint16 LE)
 *                                 Numeric:   width (byte[0x10]) + decimals (byte[0x11])
 *                                 Date/Logical: unused
 *   Offset 0x12-0x1F (14 bytes): Reserved (usually zero)
 *   Offset 0x20+:                Payload data
 *                                 Character: <length> bytes, NUL-padded
 *                                 Numeric:   8 bytes IEEE 754 double (LE)
 *                                 Date:      8 bytes IEEE 754 double (LE) = Julian Day
 *                                 Logical:   1 byte (T/F/Y/N/1/0)
 *
 * File ends with 0x1A (DOS EOF marker).
 * No file header — first record is the first variable.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <math.h>

#include "memfile.h"
#include "variables.h"
#include "exprvalue.h"

#define MEM_HEADER_SIZE  32
#define MEM_EOF_MARKER   0x1A
#define MEM_TYPE_CHAR    0xC3
#define MEM_TYPE_DATE    0xC4
#define MEM_TYPE_LOGICAL 0xCC
#define MEM_TYPE_NUMERIC 0xCE

/* ------------------------------------------------------------------ */
/* Helpers                                                             */
/* ------------------------------------------------------------------ */

static void write_le16(unsigned char *buf, unsigned short val)
{
    buf[0] = val & 0xFF;
    buf[1] = (val >> 8) & 0xFF;
}

static unsigned short read_le16(const unsigned char *buf)
{
    return (unsigned short)(buf[0] | (buf[1] << 8));
}

/* Convert a Gregorian date (year, month, day) to dBase Julian Day Number.
   dBase uses JD + 30.5 relative to the standard astronomical JD.
   E.g., 1987-01-04 → 2446800.0 (standard JD would be 2446769.5). */
static double date_to_julian(int year, int month, int day)
{
    int a = (14 - month) / 12;
    int y = year + 4800 - a;
    int m = month + 12 * a - 3;
    return (double)day - 32075 + (1461 * y) / 4 + (367 * m) / 12 - (3 * ((y + 100) / 100)) / 4 + 31.0;
}

/* Convert Julian Day Number to Gregorian date string "YYYY-MM-DD". */
static void julian_to_date_str(double jd, char *out, int out_size)
{
    int j = (int)round(jd);
    int l = j + 68569;
    int n = (4 * l) / 146097;
    l = l - (146097 * n + 3) / 4;
    int i = (4000 * (l + 1)) / 1461001;
    l = l - (1461 * i) / 4 + 31;
    int j2 = (80 * l) / 2447;
    int day = l - (2447 * j2) / 80;
    int m = j2 / 11;
    int month = j2 + 2 - 12 * m;
    int year = 100 * (n - 49) + i + m;

    snprintf(out, (size_t)out_size, "%04d-%02d-%02d", year, month, day);
}

/* Parse "YYYY-MM-DD" into year/month/day. Returns 0 on failure. */
static int parse_ymd(const char *s, int *year, int *month, int *day)
{
    int y, m, d;
    if (sscanf(s, "%d-%d-%d", &y, &m, &d) == 3) {
        *year = y; *month = m; *day = d;
        return (*year > 0 && *month >= 1 && *month <= 12 && *day >= 1 && *day <= 31);
    }
    /* Fallback: try "YYYYMMDD" */
    if ((int)strlen(s) >= 8) {
        *year  = atoi(s);
        *month = atoi(s + 4);
        *day   = atoi(s + 6);
        return (*year > 0 && *month >= 1 && *month <= 12 && *day >= 1 && *day <= 31);
    }
    return 0;
}

/* Decode logical payload byte. Returns 1=T, 0=F, -1=null/unknown. */
static int decode_logical_byte(unsigned char b)
{
    if (b == 0x54 || b == 0x74 || b == 0x59 || b == 0x79 || b == 0x31)
        return 1;   /* T t Y y 1 */
    if (b == 0x46 || b == 0x66 || b == 0x4E || b == 0x6E || b == 0x30)
        return 0;   /* F f N n 0 */
    return -1;      /* space, ?, or unknown → null */
}

/* ------------------------------------------------------------------ */
/* SAVE TO                                                             */
/* ------------------------------------------------------------------ */

int memfile_save(const char *path, const char **names, int name_count)
{
    FILE *f = fopen(path, "wb");
    if (!f)
        return -1;

    int total = vars_count();
    int saved = 0;

    for (int i = 0; i < total; i++) {
        const char *name;
        ExprValue val;

        if (!vars_get_by_index(i, &name, &val))
            continue;

        /* Check if this variable should be saved */
        if (names != NULL) {
            int found = 0;
            for (int j = 0; j < name_count; j++) {
                if (strcasecmp(name, names[j]) == 0) {
                    found = 1;
                    break;
                }
            }
            if (!found) {
                free_value(&val);
                continue;
            }
        }

        /* Build 32-byte header */
        unsigned char header[MEM_HEADER_SIZE];
        memset(header, 0, MEM_HEADER_SIZE);

        /* Name: 11 bytes, uppercase, NUL-padded */
        for (int c = 0; c < 11 && name[c]; c++)
            header[c] = (unsigned char)toupper((unsigned char)name[c]);

        /* Record ID: dBase III style (0x47D6 in upper bytes) */
        unsigned short rec_id = (unsigned short)(saved + 1);
        write_le16(header + 12, rec_id);
        header[14] = 0x47;
        header[15] = 0xD6;

        if (val.type == VAL_INTEGER || val.type == VAL_REAL) {
            /* Numeric: type byte 0xCE, width/decimals at [0x10-0x11] */
            header[0x0B] = MEM_TYPE_NUMERIC;
            header[0x10] = 16;  /* width */
            header[0x11] = 2;   /* decimals */

            /* Payload: 8-byte IEEE 754 double (LE) */
            double d = val.data.rval;
            unsigned char payload[8];
            memcpy(payload, &d, 8);

            fwrite(header, 1, MEM_HEADER_SIZE, f);
            fwrite(payload, 1, 8, f);

        } else if (val.type == VAL_DATE) {
            /* Date: type byte 0xC4, 8-byte Julian Day double */
            header[0x0B] = MEM_TYPE_DATE;

            int year, month, day;
            double jd;
            if (parse_ymd(val.data.dval, &year, &month, &day)) {
                /* Expand 2-digit years using pivot at 50:
                   00-49 → 2000-2049,  50-99 → 1950-1999 */
                if (year < 100)
                    year = year < 50 ? 2000 + year : 1900 + year;
                jd = date_to_julian(year, month, day);
            } else {
                jd = 0.0;
            }
            unsigned char payload[8];
            memcpy(payload, &jd, 8);

            fwrite(header, 1, MEM_HEADER_SIZE, f);
            fwrite(payload, 1, 8, f);

        } else if (val.type == VAL_LOGICAL) {
            /* Logical: type byte 0xCC, 1-byte payload */
            header[0x0B] = MEM_TYPE_LOGICAL;

            unsigned char payload[1];
            payload[0] = val.data.rval != 0 ? 'T' : 'F';

            fwrite(header, 1, MEM_HEADER_SIZE, f);
            fwrite(payload, 1, 1, f);

        } else {
            /* Character (including NULL): type byte 0xC3, payload length at [0x10-0x11] */
            header[0x0B] = MEM_TYPE_CHAR;

            const char *s = "";
            int slen = 0;
            if (val.type == VAL_STRING && val.data.sval) {
                s = val.data.sval;
                slen = (int)strlen(s);
            }

            write_le16(header + 0x10, (unsigned short)slen);

            fwrite(header, 1, MEM_HEADER_SIZE, f);
            if (slen > 0)
                fwrite(s, 1, (size_t)slen, f);
        }

        free_value(&val);
        saved++;
    }

    /* EOF marker */
    unsigned char eof_marker = MEM_EOF_MARKER;
    fwrite(&eof_marker, 1, 1, f);

    fclose(f);
    return saved;
}

/* ------------------------------------------------------------------ */
/* RESTORE FROM                                                        */
/* ------------------------------------------------------------------ */

int memfile_restore(const char *path, int additive)
{
    FILE *f = fopen(path, "rb");
    if (!f)
        return -1;

    if (!additive) {
        vars_shutdown();
        vars_init();
    }

    /* Read entire file into memory */
    fseek(f, 0, SEEK_END);
    long file_size = ftell(f);
    fseek(f, 0, SEEK_SET);

    if (file_size <= 0) {
        fclose(f);
        return 0;
    }

    unsigned char *data = malloc((size_t)file_size);
    if (!data) {
        fclose(f);
        return -1;
    }
    if ((long)fread(data, 1, (size_t)file_size, f) != (size_t)file_size) {
        free(data);
        fclose(f);
        return -1;
    }
    fclose(f);

    int loaded = 0;
    size_t pos = 0;

    while (pos < (size_t)file_size) {
        /* DOS EOF marker */
        if (data[pos] == MEM_EOF_MARKER)
            break;

        /* Need a full 32-byte header */
        if (pos + MEM_HEADER_SIZE > (size_t)file_size)
            break;

        unsigned char *header = data + pos;

        /* Extract name: 11 bytes, NUL-terminated */
        char name[12];
        memcpy(name, header, 11);
        name[11] = '\0';
        /* Strip trailing NULs/spaces */
        int nlen = 11;
        while (nlen > 0 && (name[nlen - 1] == '\0' || name[nlen - 1] == ' '))
            nlen--;
        name[nlen] = '\0';

        /* Type byte */
        unsigned char type_byte = header[0x0B];

        size_t payload_len = 0;
        size_t payload_start = pos + MEM_HEADER_SIZE;

        if (type_byte == MEM_TYPE_NUMERIC) {
            /* Numeric: always 8 bytes */
            payload_len = 8;

            if (payload_start + payload_len > (size_t)file_size)
                break;

            double d;
            memcpy(&d, data + payload_start, 8);

            ExprValue val;
            if (d == (double)(int)d) {
                val = val_integer((int)d);
            } else {
                val = val_real(d);
            }
            vars_set(name, &val);
            free_value(&val);

        } else if (type_byte == MEM_TYPE_CHAR) {
            /* Character: length from header[0x10-0x11] */
            payload_len = read_le16(header + 0x10);

            if (payload_start + payload_len > (size_t)file_size)
                break;

            /* Read payload, strip trailing NULs */
            char *s = malloc(payload_len + 1);
            if (s) {
                memcpy(s, data + payload_start, payload_len);
                s[payload_len] = '\0';
                /* Strip trailing NULs */
                int sl = (int)payload_len;
                while (sl > 0 && s[sl - 1] == '\0')
                    s[--sl] = '\0';

                ExprValue val = val_string(s);
                vars_set(name, &val);
                free_value(&val);
                free(s);
            }

        } else if (type_byte == MEM_TYPE_DATE) {
            /* Date: 8-byte Julian Day double → "YYYYMMDD" */
            payload_len = 8;

            if (payload_start + payload_len > (size_t)file_size)
                break;

            double jd;
            memcpy(&jd, data + payload_start, 8);

            char date_str[9];
            julian_to_date_str(jd, date_str, sizeof(date_str));

            ExprValue val = val_date(date_str);
            vars_set(name, &val);
            free_value(&val);

        } else if (type_byte == MEM_TYPE_LOGICAL) {
            /* Logical: 1-byte payload (T/F/Y/N/1/0) */
            payload_len = 1;

            if (payload_start + payload_len > (size_t)file_size)
                break;

            int logical = decode_logical_byte(data[payload_start]);
            ExprValue val = val_logical(logical >= 0 ? logical : 0);
            vars_set(name, &val);
            free_value(&val);

        } else {
            /* Unknown type — skip this record */
            break;
        }

        loaded++;
        pos = payload_start + payload_len;
    }

    free(data);
    return loaded;
}
