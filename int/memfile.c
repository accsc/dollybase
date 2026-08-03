/*
 * memfile.c — dBase III .MEM file I/O for SAVE TO / RESTORE FROM
 *
 * Writes and reads the dBase III PLUS memory variable file format.
 * Each variable is stored as a fixed 16-byte header + variable-length value data.
 * The file ends with a 0x1a (SUB) EOF marker.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "memfile.h"
#include "variables.h"
#include "exprvalue.h"

/* ------------------------------------------------------------------ */
/* Helpers                                                             */
/* ------------------------------------------------------------------ */

static void to_upper(char *dst, const char *src, int max_len)
{
    int i;
    for (i = 0; i < max_len && src[i] != '\0'; i++) {
        dst[i] = (char)toupper((unsigned char)src[i]);
    }
    for (; i < max_len; i++) {
        dst[i] = '\0';
    }
}

static void write_le16(FILE *f, unsigned short val)
{
    unsigned char buf[2];
    buf[0] = val & 0xff;
    buf[1] = (val >> 8) & 0xff;
    fwrite(buf, 1, 2, f);
}

static unsigned short read_le16(FILE *f)
{
    unsigned char buf[2];
    if (fread(buf, 1, 2, f) != 2)
        return 0;
    return (unsigned short)(buf[0] | (buf[1] << 8));
}

/* ------------------------------------------------------------------ */
/* SAVE TO                                                             */
/* ------------------------------------------------------------------ */

/* Encode an ExprValue into a buffer. Returns the type byte and sets *out_len. */
static int encode_value(const ExprValue *val, unsigned char *buf, int buf_size, int *out_len)
{
    int type_byte;
    int val_len;

    switch (val->type) {
        case VAL_NULL:
            type_byte = 0; /* C */
            buf[0] = '\0';
            val_len = 1;
            break;

        case VAL_INTEGER:
        case VAL_REAL:
            type_byte = 1; /* N */
            /* Store as double, right-aligned in a 24-byte area */
            memset(buf, 0, buf_size);
            *(double *)(buf + buf_size - 8) = val->data.rval;
            val_len = buf_size;
            break;

        case VAL_STRING: {
            type_byte = 0; /* C */
            const char *s = val->data.sval ? val->data.sval : "";
            int slen = (int)strlen(s);
            /* Length-prefixed: first byte is string length, then data */
            if (slen + 1 > buf_size)
                slen = buf_size - 1;
            buf[0] = (unsigned char)slen;
            memcpy(buf + 1, s, slen);
            memset(buf + 1 + slen, 0, buf_size - 1 - slen);
            val_len = buf_size;
            break;
        }

        case VAL_DATE:
            type_byte = 3; /* D */
            /* Store date as string "YYYY-MM-DD" (10 chars) + null */
            memset(buf, 0, buf_size);
            buf[0] = 10; /* string length */
            memcpy(buf + 1, val->data.dval, 10);
            val_len = buf_size;
            break;

        case VAL_LOGICAL:
            type_byte = 2; /* L */
            buf[0] = val->data.rval != 0 ? 0x01 : 0x00;
            val_len = 1;
            break;

        default:
            type_byte = 0;
            buf[0] = '\0';
            val_len = 1;
            break;
    }

    *out_len = val_len;
    return type_byte;
}

/* Value area size for a given type */
static int value_area_size(ValType type)
{
    switch (type) {
        case VAL_NULL:
        case VAL_LOGICAL:
            return 1;
        case VAL_STRING:
        case VAL_DATE:
        case VAL_INTEGER:
        case VAL_REAL:
            return 24; /* Standard value area size */
        default:
            return 24;
    }
}

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

        /* Write record header (16 bytes) */
        char name_buf[11];
        to_upper(name_buf, name, 10);
        fwrite(name_buf, 1, 10, f);

        int val_area_size = value_area_size(val.type);
        unsigned char *val_buf = calloc(1, (size_t)val_area_size);
        if (!val_buf) {
            free_value(&val);
            continue;
        }

        int type_byte = encode_value(&val, val_buf, val_area_size, &val_area_size);
        free_value(&val);

        fwrite(&type_byte, 1, 1, f);           /* Type */
        fwrite("\xce", 1, 1, f);               /* Flag (0xce) */
        write_le16(f, (unsigned short)val_area_size); /* Value length */
        write_le16(f, 0xd647);                 /* Magic */
        fwrite(val_buf, 1, (size_t)val_area_size, f); /* Value data */

        free(val_buf);
        saved++;
    }

    /* EOF marker */
    unsigned char eof_marker = 0x1a;
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

    /* If not additive, clear all existing variables first */
    if (!additive) {
        vars_shutdown();
        vars_init();
    }

    int loaded = 0;

    while (!feof(f)) {
        /* Read variable name (10 bytes) */
        char name_buf[11];
        size_t nread = fread(name_buf, 1, 10, f);
        if (nread == 0)
            break;
        if (nread < 10) {
            /* Partial read — might be EOF marker */
            name_buf[nread] = '\0';
            for (size_t i = nread; i < 10; i++)
                name_buf[i] = '\0';
        } else {
            name_buf[10] = '\0';
        }

        /* Check for EOF marker (0x1a) in the name area */
        int is_eof = 0;
        for (size_t i = 0; i < nread; i++) {
            if (name_buf[i] == 0x1a) {
                is_eof = 1;
                break;
            }
        }
        if (is_eof)
            break;

        /* Skip empty names (padding) */
        if (name_buf[0] == '\0') {
            /* Might be end of file or padding — try to skip to next record */
            fseek(f, 31, SEEK_CUR); /* Skip remaining 31 bytes of potential record */
            continue;
        }

        /* Read type byte */
        unsigned char type_byte;
        if (fread(&type_byte, 1, 1, f) != 1)
            break;

        /* Read flag byte */
        unsigned char flag_byte;
        if (fread(&flag_byte, 1, 1, f) != 1)
            break;

        /* Read value length */
        unsigned short val_len = read_le16(f);
        if (val_len == 0 || val_len > 4096) {
            /* Invalid length — skip this record */
            continue;
        }

        /* Read magic */
        unsigned short magic = read_le16(f);
        (void)magic; /* We accept both 0xd647 and 0xca66 */

        /* Read value data */
        unsigned char *val_buf = malloc((size_t)val_len);
        if (!val_buf)
            break;
        if (fread(val_buf, 1, (size_t)val_len, f) != val_len) {
            free(val_buf);
            break;
        }

        /* Decode value based on type */
        ExprValue val;
        switch (type_byte) {
            case 0: { /* Character */
                /* Length-prefixed string: first byte is length */
                int slen = val_buf[0];
                if (slen > (int)val_len - 1)
                    slen = (int)val_len - 1;
                char *s = malloc((size_t)(slen + 1));
                if (s) {
                    memcpy(s, val_buf + 1, (size_t)slen);
                    s[slen] = '\0';
                    val = val_string(s);
                    free(s);
                } else {
                    val = val_string("");
                }
                break;
            }

            case 1: { /* Numeric — double right-aligned in value area */
                double d = 0.0;
                if (val_len >= 8) {
                    memcpy(&d, val_buf + val_len - 8, 8);
                }
                val = val_real(d);
                break;
            }

            case 2: { /* Logical */
                int t = (val_len > 0 && val_buf[0] != 0x00) ? 1 : 0;
                val = val_logical(t);
                break;
            }

            case 3: { /* Date */
                /* Date stored as length-prefixed string "YYYY-MM-DD" */
                char date_str[12] = "0000-00-00";
                int slen = val_buf[0];
                if (slen > 10)
                    slen = 10;
                if (slen > 0 && slen <= (int)val_len - 1) {
                    memcpy(date_str, val_buf + 1, (size_t)slen);
                }
                date_str[10] = '\0';
                val = val_date(date_str);
                break;
            }

            default:
                val = val_null();
                break;
        }

        free(val_buf);

        /* Store variable */
        vars_set(name_buf, &val);
        free_value(&val);
        loaded++;
    }

    fclose(f);
    return loaded;
}
