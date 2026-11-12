// cg50_compat.h -- force-included into every engine source file
// (-include cg50_compat.h). Routes the few libc calls that fxlibc may not
// provide (or that make no sense on a calculator) to our own versions.
#ifndef CG50_COMPAT_H
#define CG50_COMPAT_H

#include <stdarg.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct cg_file CG_FILE;

CG_FILE *cg_fopen(const char *path, const char *mode);
int cg_fclose(CG_FILE *f);
size_t cg_fread(void *buf, size_t size, size_t n, CG_FILE *f);
size_t cg_fwrite(const void *buf, size_t size, size_t n, CG_FILE *f);
int cg_fseek(CG_FILE *f, long off, int whence);
long cg_ftell(CG_FILE *f);
int cg_fflush(CG_FILE *f);
int cg_vfprintf(CG_FILE *f, const char *fmt, va_list ap);
int cg_fprintf(CG_FILE *f, const char *fmt, ...);
int cg_remove(const char *path);
int cg_rename(const char *from, const char *to);
int cg_mkdir(const char *path, ...);
int cg_system(const char *cmd);
int cg_strcasecmp(const char *a, const char *b);
int cg_strncasecmp(const char *a, const char *b, size_t n);
char *cg_strdup(const char *s);
int cg_sscanf_int(const char *s, const char *fmt, ...);
double cg_atof(const char *s);
char *cg_getenv(const char *name);

extern CG_FILE *cg_stdout;
extern CG_FILE *cg_stderr;

#ifdef __cplusplus
}
#endif

// Only the engine sources get the remapping; the platform files define the
// cg_* functions themselves and set CG50_NO_REMAP.
#ifndef CG50_NO_REMAP
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#undef FILE
#undef stdout
#undef stderr
#define FILE CG_FILE
#define stdout cg_stdout
#define stderr cg_stderr
#define fopen cg_fopen
#define fclose cg_fclose
#define fread cg_fread
#define fwrite cg_fwrite
#define fseek cg_fseek
#define ftell cg_ftell
#define fflush cg_fflush
#define vfprintf cg_vfprintf
#define fprintf cg_fprintf
#define remove cg_remove
#define rename cg_rename
#define mkdir cg_mkdir
#define system cg_system
#define strcasecmp cg_strcasecmp
#define strncasecmp cg_strncasecmp
#define strdup cg_strdup
#define atof cg_atof
#define getenv cg_getenv
#define bzero(p, n) memset((p), 0, (n))
#endif

#endif
