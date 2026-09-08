#include "je/je_loader.h"
#include "je/je_file.h"
#include "je/je_error.h"
#include "je/je_text.h"
#include "je/je_exec.h"
#include <string.h>
#include <ctype.h>

JEDetectResult je_detect_file(const char *path) {
    JEFileBuffer b = {0};
    if (!path || je_file_read_all(path, &b) != JE_OK) return JE_DETECT_NO;

    JEDetectResult result = JE_DETECT_NO;
    const int magic = je_has_magic((const char *)b.data, b.size);
    const int ext = je_has_jxj_extension(path);
    JEHeader header;
    const int valid_header = (je_parse_header((const char *)b.data, b.size, &header, NULL) == JE_OK);

    if (magic && valid_header) {
        result = JE_DETECT_MAGIC;
    } else if (ext && valid_header) {
        result = JE_DETECT_JXJ;
    }

    je_file_buffer_free(&b);
    return result;
}

JEStatus je_load_and_run_code(const char *path, int *exit_code) {
    JEFileBuffer b = {0};
    JEHeader header;
    size_t body = 0;
    if (!path) return JE_ERR_INVALID_ARGUMENT;
    JEStatus st = je_file_read_all(path, &b);
    if (st != JE_OK) return st;
    st = je_parse_header((const char *)b.data, b.size, &header, &body);
    if (st != JE_OK) {
        je_file_buffer_free(&b);
        je_set_error("invalid JE header");
        return st;
    }
    (void)header;
    if (body > b.size) { je_file_buffer_free(&b); return JE_ERR_FORMAT; }
    const char *text = (const char *)b.data + body;
    size_t text_size = b.size - body;
    const char *start = text;
    const char *code = NULL;
    const char *end = NULL;
    const char *limit = text + text_size;
    int in_string = 0, escaped = 0, in_comment = 0;
    for (const char *q = start; q + 4 <= limit; ++q) {
        unsigned char c = (unsigned char)*q;
        if (in_comment) {
            if (c == '\n') in_comment = 0;
            continue;
        }
        if (in_string) {
            if (escaped) escaped = 0;
            else if (c == '\\') escaped = 1;
            else if (c == '"') in_string = 0;
            continue;
        }
        if (c == '"') { in_string = 1; continue; }
        if (c == '#') { in_comment = 1; continue; }
        int left_ok = (q == start) || !(isalnum((unsigned char)q[-1]) || q[-1] == '_' || q[-1] == '-');
        if (!left_ok || memcmp(q, "CODE", 4) != 0) continue;
        const char *r = q + 4;
        int right_ok = (r == limit) || !(isalnum((unsigned char)*r) || *r == '_' || *r == '-');
        if (!right_ok) continue;
        while (r < limit && isspace((unsigned char)*r)) ++r;
        if (r < limit && *r == '{') { code = r + 1; break; }
    }
    if (!code) {
        je_file_buffer_free(&b);
        je_set_error("JE file has no CODE block");
        return JE_ERR_FORMAT;
    }
    int depth = 1;
    in_string = 0; escaped = 0;
    for (const char *q = code; q < limit; ++q) {
        unsigned char c = (unsigned char)*q;
        if (in_string) {
            if (escaped) escaped = 0;
            else if (c == '\\') escaped = 1;
            else if (c == '"') in_string = 0;
            continue;
        }
        if (c == '"') { in_string = 1; continue; }
        if (c == '{') ++depth;
        else if (c == '}' && --depth == 0) { end = q; break; }
    }
    if (!end || depth != 0 || in_string) {
        je_file_buffer_free(&b);
        je_set_error("JE CODE block is unterminated");
        return JE_ERR_PARSE;
    }
    JEExecContext ctx;
    je_exec_init(&ctx);
    st = je_exec_text(&ctx, code, (size_t)(end - code));
    je_file_buffer_free(&b);
    if (st != JE_OK) return st;
    if (exit_code) *exit_code = ctx.exit_code & 0xFF;
    return JE_OK;
}

JEStatus je_load_and_run(const char *path) {
    return je_load_and_run_code(path, NULL);
}
