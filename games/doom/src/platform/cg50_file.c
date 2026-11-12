// cg50_file.c -- file I/O for the Doom port, built on gint's BFile API.
//
//  * A tiny FILE replacement (CG_FILE) used by config/savegame code. Files are
//    read fully into RAM on open and written back on close, which suits the
//    Casio filesystem (files must be created with a fixed size).
//  * The WAD file class used by w_wad.c. The WAD is large, so it is read
//    straight from storage memory in lumps and never fully loaded.
//
// All paths are relative to the storage-memory root: "\\fls0\".

#define CG50_NO_REMAP
#include "cg50_compat.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include <gint/bfile.h>

#include "cg50_platform.h"

#include "w_file.h"
#include "z_zone.h"

#define MAX_PATH 96

// ---------------------------------------------------------------------------
// Path helpers
// ---------------------------------------------------------------------------

// Build a UTF-16 "\\fls0\name" path. `out` must hold MAX_PATH entries.
static int make_path(uint16_t *out, const char *path)
{
    static const char root[] = "\\\\fls0\\";
    int n = 0;
    const char *p;

    if (path[0] != '\\' && path[0] != '/')
    {
        for (p = root; *p && n < MAX_PATH - 1; ++p)
            out[n++] = (uint16_t) *p;
    }

    for (p = path; *p && n < MAX_PATH - 1; ++p)
        out[n++] = (uint16_t) (*p == '/' ? '\\' : *p);

    out[n] = 0;
    return n;
}

// ---------------------------------------------------------------------------
// CG_FILE
// ---------------------------------------------------------------------------

struct cg_file
{
    int sink;           // 1 = stdout/stderr, text goes to cg_log()
    int writing;
    uint8_t *data;
    size_t size, cap, pos;
    uint16_t path[MAX_PATH];
};

static struct cg_file stdout_file = { 1, 1, NULL, 0, 0, 0, { 0 } };
static struct cg_file stderr_file = { 1, 1, NULL, 0, 0, 0, { 0 } };
CG_FILE *cg_stdout = &stdout_file;
CG_FILE *cg_stderr = &stderr_file;

CG_FILE *cg_fopen(const char *path, const char *mode)
{
    CG_FILE *f;

    f = calloc(1, sizeof(*f));
    if (f == NULL)
        return NULL;

    make_path(f->path, path);

    if (mode[0] == 'r')
    {
        int fd = BFile_Open(f->path, BFile_ReadOnly);
        int size;

        if (fd < 0)
        {
            free(f);
            return NULL;
        }

        size = BFile_Size(fd);
        if (size < 0)
            size = 0;

        f->data = malloc(size ? size : 1);
        if (f->data == NULL)
        {
            BFile_Close(fd);
            free(f);
            return NULL;
        }

        // Read position -1 would mean "continue"; we always read from 0.
        if (size > 0 && BFile_Read(fd, f->data, size, 0) < 0)
        {
            BFile_Close(fd);
            free(f->data);
            free(f);
            return NULL;
        }

        BFile_Close(fd);
        f->size = f->cap = (size_t) size;
    }
    else
    {
        f->writing = 1;
        f->cap = 1024;
        f->data = malloc(f->cap);
        if (f->data == NULL)
        {
            free(f);
            return NULL;
        }
    }

    return f;
}

int cg_fclose(CG_FILE *f)
{
    int ret = 0;

    if (f == NULL || f->sink)
        return 0;

    if (f->writing)
    {
        int size = (int) f->size;
        int fd;

        BFile_Remove(f->path);
        if (BFile_Create(f->path, BFile_File, &size) < 0)
        {
            ret = -1;
        }
        else
        {
            fd = BFile_Open(f->path, BFile_WriteOnly);
            if (fd < 0 || BFile_Write(fd, f->data, (int) f->size) < 0)
                ret = -1;
            if (fd >= 0)
                BFile_Close(fd);
        }
    }

    free(f->data);
    free(f);
    return ret;
}

size_t cg_fread(void *buf, size_t size, size_t n, CG_FILE *f)
{
    size_t want, avail;

    if (f == NULL || f->sink || f->writing || size == 0)
        return 0;

    want = size * n;
    avail = f->size - f->pos;
    if (want > avail)
        want = avail - (avail % size);

    memcpy(buf, f->data + f->pos, want);
    f->pos += want;
    return want / size;
}

size_t cg_fwrite(const void *buf, size_t size, size_t n, CG_FILE *f)
{
    size_t bytes = size * n;

    if (f == NULL || f->sink || !f->writing || bytes == 0)
        return 0;

    if (f->pos + bytes > f->cap)
    {
        size_t cap = f->cap * 2;
        uint8_t *d;
        while (cap < f->pos + bytes)
            cap *= 2;
        d = realloc(f->data, cap);
        if (d == NULL)
            return 0;
        f->data = d;
        f->cap = cap;
    }

    memcpy(f->data + f->pos, buf, bytes);
    f->pos += bytes;
    if (f->pos > f->size)
        f->size = f->pos;
    return n;
}

int cg_fseek(CG_FILE *f, long off, int whence)
{
    long base;

    if (f == NULL || f->sink)
        return -1;

    switch (whence)
    {
        case 0: base = 0; break;                 // SEEK_SET
        case 1: base = (long) f->pos; break;     // SEEK_CUR
        case 2: base = (long) f->size; break;    // SEEK_END
        default: return -1;
    }

    if (base + off < 0 || (size_t) (base + off) > f->size)
        return -1;

    f->pos = (size_t) (base + off);
    return 0;
}

long cg_ftell(CG_FILE *f)
{
    return (f == NULL || f->sink) ? -1 : (long) f->pos;
}

int cg_fflush(CG_FILE *f)
{
    (void) f;
    return 0;
}

int cg_vfprintf(CG_FILE *f, const char *fmt, va_list ap)
{
    char tmp[256];
    int n = vsnprintf(tmp, sizeof(tmp), fmt, ap);

    if (n < 0)
        return n;

    if (f != NULL && f->sink)
    {
        cg_log(tmp);
        return n;
    }

    cg_fwrite(tmp, 1, strlen(tmp), f);
    return n;
}

int cg_fprintf(CG_FILE *f, const char *fmt, ...)
{
    va_list ap;
    int n;

    va_start(ap, fmt);
    n = cg_vfprintf(f, fmt, ap);
    va_end(ap);
    return n;
}

int cg_remove(const char *path)
{
    uint16_t p[MAX_PATH];
    make_path(p, path);
    return BFile_Remove(p) < 0 ? -1 : 0;
}

int cg_rename(const char *from, const char *to)
{
    // Not worth supporting: Doom only uses it for savegame temp files.
    (void) from;
    (void) to;
    return -1;
}

int cg_mkdir(const char *path, ...)
{
    (void) path;
    return 0;
}

int cg_system(const char *cmd)
{
    (void) cmd;
    return -1;
}

char *cg_getenv(const char *name)
{
    (void) name;
    return NULL;
}

// ---------------------------------------------------------------------------
// String helpers that fxlibc may or may not have
// ---------------------------------------------------------------------------

int cg_strcasecmp(const char *a, const char *b)
{
    for (;; ++a, ++b)
    {
        int d = tolower((unsigned char) *a) - tolower((unsigned char) *b);
        if (d != 0 || *a == '\0')
            return d;
    }
}

int cg_strncasecmp(const char *a, const char *b, size_t n)
{
    for (; n > 0; ++a, ++b, --n)
    {
        int d = tolower((unsigned char) *a) - tolower((unsigned char) *b);
        if (d != 0 || *a == '\0')
            return d;
    }
    return 0;
}

char *cg_strdup(const char *s)
{
    size_t n = strlen(s) + 1;
    char *d = malloc(n);
    if (d != NULL)
        memcpy(d, s, n);
    return d;
}

double cg_atof(const char *s)
{
    // Minimal decimal parser; only used for mouse_acceleration in configs.
    double v = 0.0, scale = 1.0;
    int neg = 0, seen_dot = 0;

    while (*s == ' ' || *s == '\t')
        ++s;
    if (*s == '-' || *s == '+')
        neg = (*s++ == '-');

    for (; *s; ++s)
    {
        if (*s == '.' && !seen_dot)
        {
            seen_dot = 1;
        }
        else if (*s >= '0' && *s <= '9')
        {
            v = v * 10.0 + (*s - '0');
            if (seen_dot)
                scale *= 10.0;
        }
        else
        {
            break;
        }
    }

    v /= scale;
    return neg ? -v : v;
}

// ---------------------------------------------------------------------------
// WAD file class (replaces w_file_stdc.c)
// ---------------------------------------------------------------------------

typedef struct
{
    wad_file_t wad;
    int fd;
} cg_wad_file_t;

extern wad_file_class_t stdc_wad_file;

static wad_file_t *W_CG_OpenFile(char *path)
{
    uint16_t p[MAX_PATH];
    cg_wad_file_t *result;
    int fd, size;

    make_path(p, path);
    fd = BFile_Open(p, BFile_ReadOnly);
    if (fd < 0)
        return NULL;

    size = BFile_Size(fd);
    if (size <= 0)
    {
        BFile_Close(fd);
        return NULL;
    }

    result = Z_Malloc(sizeof(cg_wad_file_t), PU_STATIC, 0);
    result->wad.file_class = &stdc_wad_file;
    result->wad.mapped = NULL;
    result->wad.length = (unsigned int) size;
    result->fd = fd;
    return &result->wad;
}

static void W_CG_CloseFile(wad_file_t *wad)
{
    cg_wad_file_t *w = (cg_wad_file_t *) wad;

    BFile_Close(w->fd);
    Z_Free(w);
}

static size_t W_CG_Read(wad_file_t *wad, unsigned int offset,
                        void *buffer, size_t buffer_len)
{
    cg_wad_file_t *w = (cg_wad_file_t *) wad;
    int got;

    // The last argument is an absolute read position; see bfile.h.
    got = BFile_Read(w->fd, buffer, (int) buffer_len, (int) offset);
    return got < 0 ? 0 : (size_t) got;
}

wad_file_class_t stdc_wad_file =
{
    W_CG_OpenFile,
    W_CG_CloseFile,
    W_CG_Read,
};
