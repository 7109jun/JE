#include "je/je_process.h"
#include "je/je_error.h"
#include <stddef.h>

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

static char *quote_arg(const char *s) {
    if (!s) return NULL;
    size_t n = strlen(s), extra = 2, slashes = 0;
    for (size_t i = 0; i < n; ++i) {
        if (s[i] == '\\') {
            ++slashes;
        } else {
            if (s[i] == '"') {
                if (extra == SIZE_MAX) return NULL;
                ++extra;
            }
            slashes = 0;
        }
    }
    if (n > (SIZE_MAX - extra - 8u) / 2u) return NULL;
    size_t cap = n * 2u + extra + 8u;
    char *out = (char *)malloc(cap);
    if (!out) return NULL;
    size_t j = 0;
    out[j++] = '"';
    slashes = 0;
    for (size_t i = 0; i < n; ++i) {
        if (s[i] == '\\') {
            ++slashes;
            continue;
        }
        if (s[i] == '"') {
            for (size_t k = 0; k < slashes * 2u + 1u; ++k) out[j++] = '\\';
            out[j++] = '"';
            slashes = 0;
            continue;
        }
        while (slashes-- > 0) out[j++] = '\\';
        slashes = 0;
        out[j++] = s[i];
    }
    for (size_t k = 0; k < slashes * 2u; ++k) out[j++] = '\\';
    out[j++] = '"';
    out[j] = '\0';
    return out;
}

JEStatus je_process_launch(const char *path, const char *const *argv, int *exit_code) {
    if (!path || !*path) return JE_ERR_INVALID_ARGUMENT;
    char *cmd = quote_arg(path);
    if (!cmd) return JE_ERR_MEMORY;
    if (argv) {
        for (size_t i = 0; argv[i]; ++i) {
            char *q = quote_arg(argv[i]);
            if (!q) { free(cmd); return JE_ERR_MEMORY; }
            size_t a = strlen(cmd), b = strlen(q);
            if (a > SIZE_MAX - b - 2u || a + b + 1u > 32766u) {
                free(q); free(cmd);
                je_set_error("Windows command line is too long");
                return JE_ERR_INVALID_ARGUMENT;
            }
            char *tmp = (char *)realloc(cmd, a + b + 2u);
            if (!tmp) { free(q); free(cmd); return JE_ERR_MEMORY; }
            cmd = tmp;
            cmd[a] = ' ';
            memcpy(cmd + a + 1u, q, b + 1u);
            free(q);
        }
    }
    if (strlen(cmd) > 32766u) {
        free(cmd);
        je_set_error("Windows command line is too long");
        return JE_ERR_INVALID_ARGUMENT;
    }
    STARTUPINFOA si;
    PROCESS_INFORMATION pi;
    ZeroMemory(&si, sizeof(si));
    si.cb = sizeof(si);
    ZeroMemory(&pi, sizeof(pi));
    BOOL ok = CreateProcessA(NULL, cmd, NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi);
    free(cmd);
    if (!ok) {
        je_set_error("CreateProcessA failed");
        return JE_ERR_IO;
    }
    DWORD wait_result = WaitForSingleObject(pi.hProcess, INFINITE);
    if (wait_result != WAIT_OBJECT_0) {
        CloseHandle(pi.hThread);
        CloseHandle(pi.hProcess);
        je_set_error("WaitForSingleObject failed");
        return JE_ERR_IO;
    }
    DWORD code = 0;
    if (!GetExitCodeProcess(pi.hProcess, &code)) {
        CloseHandle(pi.hThread);
        CloseHandle(pi.hProcess);
        je_set_error("GetExitCodeProcess failed");
        return JE_ERR_IO;
    }
    CloseHandle(pi.hThread);
    CloseHandle(pi.hProcess);
    if (exit_code) *exit_code = (int)(code & 0xFFu);
    return JE_OK;
}
#else
JEStatus je_process_launch(const char *path, const char *const *argv, int *exit_code) {
    (void)path; (void)argv; (void)exit_code; je_set_error("native Windows process launch is unavailable on this platform"); return JE_ERR_UNSUPPORTED;
}
#endif
