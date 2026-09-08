#include "je/je_runtime.h"
#include "je/je_error.h"
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <limits.h>
#if defined(_WIN32)
#include <io.h>
#endif

#define JE_RUNTIME_MAX_STRING 1048576u

static int mul_overflow(size_t a, size_t b, size_t *out) {
    if (a != 0 && b > SIZE_MAX / a) return 1;
    *out = a * b;
    return 0;
}

static int remember_alloc(JEExecContext *ctx, void *ptr, size_t size) {
    if (!ptr) return 0;
    if (ctx->alloc_count >= 128u) return 0;
    ctx->allocs[ctx->alloc_count] = ptr;
    ctx->alloc_sizes[ctx->alloc_count] = size;
    ++ctx->alloc_count;
    return 1;
}

static size_t find_alloc(const JEExecContext *ctx, const void *ptr) {
    if (!ctx || !ptr) return SIZE_MAX;
    for (size_t i = 0; i < ctx->alloc_count; ++i) {
        uintptr_t base = (uintptr_t)ctx->allocs[i];
        size_t n = ctx->alloc_sizes[i];
        uintptr_t p = (uintptr_t)ptr;
        if (p >= base && (size_t)(p - base) <= n) return i;
    }
    return SIZE_MAX;
}

static int validate_range(const JEExecContext *ctx, const void *ptr, size_t len, int allow_end) {
    size_t i = find_alloc(ctx, ptr);
    if (i == SIZE_MAX) return 0;
    uintptr_t base = (uintptr_t)ctx->allocs[i];
    uintptr_t p = (uintptr_t)ptr;
    size_t off = (size_t)(p - base);
    size_t size = ctx->alloc_sizes[i];
    if (off > size) return 0;
    if (len > size - off) return 0;
    if (!allow_end && off == size) return 0;
    return 1;
}

static int validate_cstring(const JEExecContext *ctx, const char *s) {
    size_t i = find_alloc(ctx, s);
    if (i == SIZE_MAX) return 0;
    uintptr_t base = (uintptr_t)ctx->allocs[i];
    uintptr_t p = (uintptr_t)s;
    size_t off = (size_t)(p - base);
    size_t size = ctx->alloc_sizes[i];
    if (off >= size) return 0;
    size_t remain = size - off;
    if (remain > JE_RUNTIME_MAX_STRING) remain = JE_RUNTIME_MAX_STRING;
    return memchr(s, '\0', remain) != NULL;
}

static void forget_alloc(JEExecContext *ctx, size_t i) {
    if (!ctx || i >= ctx->alloc_count) return;
    ctx->allocs[i] = ctx->allocs[ctx->alloc_count - 1u];
    ctx->alloc_sizes[i] = ctx->alloc_sizes[ctx->alloc_count - 1u];
    --ctx->alloc_count;
}

static int remember_file(JEExecContext *ctx, FILE *f) {
    if (!f || ctx->file_count >= 64u) return 0;
    ctx->files[ctx->file_count++] = f;
    return 1;
}

static size_t find_file(const JEExecContext *ctx, FILE *f) {
    if (!ctx || !f) return SIZE_MAX;
    for (size_t i = 0; i < ctx->file_count; ++i)
        if (ctx->files[i] == f) return i;
    return SIZE_MAX;
}

static void forget_file(JEExecContext *ctx, size_t i) {
    if (!ctx || i >= ctx->file_count) return;
    ctx->files[i] = ctx->files[ctx->file_count - 1u];
    --ctx->file_count;
}

static int reg_ptr(const JEExecContext *ctx, int r, void **out) {
    if (!ctx || !out || r < 0 || r > 7) return 0;
    *out = (void *)(uintptr_t)ctx->regs[r];
    return *out != NULL;
}

static int reg_size(const JEExecContext *ctx, int r, size_t *out) {
    if (!ctx || !out || r < 0 || r > 7) return 0;
    if (ctx->regs[r] > (uint64_t)SIZE_MAX) return 0;
    *out = (size_t)ctx->regs[r];
    return 1;
}

void je_runtime_cleanup(JEExecContext *ctx) {
    if (!ctx) return;
    while (ctx->file_count) {
        FILE *f = (FILE *)ctx->files[ctx->file_count - 1u];
        --ctx->file_count;
        (void)fclose(f);
    }
    while (ctx->alloc_count) {
        void *p = ctx->allocs[ctx->alloc_count - 1u];
        --ctx->alloc_count;
        free(p);
    }
}

JEStatus je_runtime_call(JEExecContext *ctx, const char *name) {
    if (!ctx || !name) return JE_ERR_INVALID_ARGUMENT;

    if (strcmp(name, "IMPORT.malloc") == 0 || strcmp(name, "IMPORT.CRT.malloc") == 0) {
        size_t size;
        if (!reg_size(ctx, 0, &size)) { je_set_error("malloc size must be a valid size_t"); return JE_ERR_PARSE; }
        void *p = malloc(size ? size : 1u);
        if (!p) { je_set_error("malloc failed"); return JE_ERR_MEMORY; }
        if (!remember_alloc(ctx, p, size ? size : 1u)) { free(p); je_set_error("too many JE allocations"); return JE_ERR_MEMORY; }
        ctx->regs[0] = (uint64_t)(uintptr_t)p;
        ctx->zero = (p == NULL);
        return JE_OK;
    }

    if (strcmp(name, "IMPORT.calloc") == 0 || strcmp(name, "IMPORT.CRT.calloc") == 0) {
        size_t count, size, total;
        if (!reg_size(ctx, 0, &count) || !reg_size(ctx, 1, &size) || mul_overflow(count, size, &total)) {
            je_set_error("invalid calloc arguments"); return JE_ERR_PARSE;
        }
        void *p = calloc(count ? count : 1u, size ? size : 1u);
        if (!p) { je_set_error("calloc failed"); return JE_ERR_MEMORY; }
        if (!remember_alloc(ctx, p, total ? total : 1u)) { free(p); je_set_error("too many JE allocations"); return JE_ERR_MEMORY; }
        ctx->regs[0] = (uint64_t)(uintptr_t)p;
        return JE_OK;
    }

    if (strcmp(name, "IMPORT.realloc") == 0 || strcmp(name, "IMPORT.CRT.realloc") == 0) {
        void *oldp = (void *)(uintptr_t)ctx->regs[0];
        size_t new_size;
        size_t idx = oldp ? find_alloc(ctx, oldp) : SIZE_MAX;
        if (!reg_size(ctx, 1, &new_size) || (oldp && idx == SIZE_MAX)) {
            je_set_error("invalid realloc arguments"); return JE_ERR_PARSE;
        }
        void *p = realloc(oldp, new_size ? new_size : 1u);
        if (!p) { je_set_error("realloc failed"); return JE_ERR_MEMORY; }
        if (idx == SIZE_MAX) {
            if (!remember_alloc(ctx, p, new_size ? new_size : 1u)) { free(p); je_set_error("too many JE allocations"); return JE_ERR_MEMORY; }
        } else {
            ctx->allocs[idx] = p;
            ctx->alloc_sizes[idx] = new_size ? new_size : 1u;
        }
        ctx->regs[0] = (uint64_t)(uintptr_t)p;
        return JE_OK;
    }

    if (strcmp(name, "IMPORT.free") == 0 || strcmp(name, "IMPORT.CRT.free") == 0) {
        void *p = NULL;
        if (!reg_ptr(ctx, 0, &p)) { je_set_error("free expects a non-null pointer"); return JE_ERR_PARSE; }
        size_t idx = find_alloc(ctx, p);
        if (idx == SIZE_MAX) { je_set_error("free received a pointer not owned by JE runtime"); return JE_ERR_PARSE; }
        free(p); forget_alloc(ctx, idx); ctx->regs[0] = 0; ctx->zero = 1; return JE_OK;
    }

    if (strcmp(name, "IMPORT.memcpy") == 0 || strcmp(name, "IMPORT.CRT.memcpy") == 0 ||
        strcmp(name, "IMPORT.memset") == 0 || strcmp(name, "IMPORT.CRT.memset") == 0) {
        void *dst = (void *)(uintptr_t)ctx->regs[0];
        void *src_or_value = (void *)(uintptr_t)ctx->regs[1];
        size_t n;
        if (!reg_size(ctx, 2, &n)) { je_set_error("invalid memory operation length"); return JE_ERR_PARSE; }
        if (!validate_range(ctx, dst, n, 0)) { je_set_error("destination is outside a JE allocation"); return JE_ERR_PARSE; }
        if (strstr(name, "memcpy") != NULL) {
            if (!validate_range(ctx, src_or_value, n, 0)) { je_set_error("source is outside a JE allocation"); return JE_ERR_PARSE; }
            memcpy(dst, src_or_value, n);
        } else {
            memset(dst, (int)(ctx->regs[1] & 0xFFu), n);
        }
        ctx->regs[0] = (uint64_t)(uintptr_t)dst;
        return JE_OK;
    }

    if (strcmp(name, "IMPORT.fopen") == 0 || strcmp(name, "IMPORT.CRT.fopen") == 0) {
        char *path = (char *)(uintptr_t)ctx->regs[0];
        char *mode = (char *)(uintptr_t)ctx->regs[1];
        if (!validate_cstring(ctx, path) || !validate_cstring(ctx, mode)) {
            je_set_error("fopen expects valid JE-owned NUL-terminated strings"); return JE_ERR_PARSE;
        }
        FILE *f = fopen(path, mode);
        if (!f) { je_set_error("fopen failed"); return JE_ERR_IO; }
        if (!remember_file(ctx, f)) { fclose(f); je_set_error("too many JE file handles"); return JE_ERR_IO; }
        ctx->regs[0] = (uint64_t)(uintptr_t)f;
        return JE_OK;
    }

    if (strcmp(name, "IMPORT.fclose") == 0 || strcmp(name, "IMPORT.CRT.fclose") == 0) {
        FILE *f = (FILE *)(uintptr_t)ctx->regs[0];
        size_t idx = find_file(ctx, f);
        if (idx == SIZE_MAX) { je_set_error("fclose received an unknown file handle"); return JE_ERR_PARSE; }
        if (fclose(f) != 0) { je_set_error("fclose failed"); return JE_ERR_IO; }
        forget_file(ctx, idx); ctx->regs[0] = 0; ctx->zero = 1; return JE_OK;
    }

    if (strcmp(name, "IMPORT.fread") == 0 || strcmp(name, "IMPORT.CRT.fread") == 0 ||
        strcmp(name, "IMPORT.fwrite") == 0 || strcmp(name, "IMPORT.CRT.fwrite") == 0) {
        void *buffer = (void *)(uintptr_t)ctx->regs[0];
        size_t size, count;
        FILE *f = (FILE *)(uintptr_t)ctx->regs[3];
        if (!reg_size(ctx, 1, &size) || !reg_size(ctx, 2, &count) || find_file(ctx, f) == SIZE_MAX) {
            je_set_error("invalid fread/fwrite arguments"); return JE_ERR_PARSE;
        }
        size_t total;
        if (mul_overflow(size, count, &total) || !validate_range(ctx, buffer, total, total == 0)) {
            je_set_error("buffer is outside a JE allocation"); return JE_ERR_PARSE;
        }
        size_t done;
        if (strstr(name, "fread") != NULL) done = fread(buffer, size, count, f);
        else done = fwrite(buffer, size, count, f);
        ctx->regs[0] = (uint64_t)done;
        ctx->zero = (done == 0);
        return JE_OK;
    }

    if (strcmp(name, "IMPORT.fseek") == 0 || strcmp(name, "IMPORT.CRT.fseek") == 0) {
        FILE *f = (FILE *)(uintptr_t)ctx->regs[0];
        if (find_file(ctx, f) == SIZE_MAX) { je_set_error("fseek received an unknown file handle"); return JE_ERR_PARSE; }
        int origin = (int)ctx->regs[2];
        if (origin != SEEK_SET && origin != SEEK_CUR && origin != SEEK_END) { je_set_error("invalid fseek origin"); return JE_ERR_PARSE; }
#if defined(_WIN32)
        int r = _fseeki64(f, (long long)(int64_t)ctx->regs[1], origin);
#else
        if ((int64_t)ctx->regs[1] > (int64_t)LONG_MAX ||
            (int64_t)ctx->regs[1] < (int64_t)LONG_MIN) {
            je_set_error("fseek offset is outside host long range");
            return JE_ERR_PARSE;
        }
        int r = fseek(f, (long)(int64_t)ctx->regs[1], origin);
#endif
        ctx->regs[0] = (uint64_t)(r == 0 ? 0 : 1);
        ctx->zero = (r == 0);
        return r == 0 ? JE_OK : JE_ERR_IO;
    }

    if (strcmp(name, "IMPORT.fflush") == 0 || strcmp(name, "IMPORT.CRT.fflush") == 0) {
        FILE *f = (FILE *)(uintptr_t)ctx->regs[0];
        if (find_file(ctx, f) == SIZE_MAX) { je_set_error("fflush received an unknown file handle"); return JE_ERR_PARSE; }
        int r = fflush(f);
        ctx->regs[0] = (uint64_t)(r == 0 ? 0 : 1);
        ctx->zero = (r == 0);
        return r == 0 ? JE_OK : JE_ERR_IO;
    }

    if (strncmp(name, "IMPORT.", 7) == 0) {
        je_set_error("unsupported JE runtime import");
        return JE_ERR_UNSUPPORTED;
    }

    return JE_ERR_UNSUPPORTED;
}
