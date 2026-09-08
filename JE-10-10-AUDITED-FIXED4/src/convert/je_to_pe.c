#include "je/je_convert.h"
#include "je/je_file.h"
#include "je/je_error.h"
#include "je/je_text.h"
#include "je/je_pe.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

static int hexval(int c) {
    if (c >= '0' && c <= '9') return c - '0';
    if (c >= 'A' && c <= 'F') return c - 'A' + 10;
    if (c >= 'a' && c <= 'f') return c - 'a' + 10;
    return -1;
}

JEStatus je_je_to_pe(const char *je_path, const char *pe_path) {
    JEFileBuffer in = {0};
    JEHeader header;
    size_t body = 0;
    if (!je_path || !pe_path) return JE_ERR_INVALID_ARGUMENT;
    JEStatus st = je_file_read_all(je_path, &in);
    if (st != JE_OK) return st;
    st = je_parse_header((const char *)in.data, in.size, &header, &body);
    if (st != JE_OK) { je_file_buffer_free(&in); return st; }

    const char *tag = "PE_RAW_HEX";
    const char *p = (const char *)in.data;
    const char *end = p + in.size;
    const char *b = NULL, *e = NULL;
    while (p < end) {
        if ((size_t)(end - p) >= strlen(tag) && memcmp(p, tag, strlen(tag)) == 0) {
            const char *q = p + strlen(tag);
            while (q < end && isspace((unsigned char)*q)) ++q;
            if (q < end && *q == '{') { b = q + 1; break; }
        }
        ++p;
    }
    if (!b) { je_file_buffer_free(&in); je_set_error("JE has no PE_RAW_HEX section"); return JE_ERR_FORMAT; }
    for (p = b; p < end; ++p) if (*p == '}') { e = p; break; }
    if (!e) { je_file_buffer_free(&in); je_set_error("unterminated PE_RAW_HEX section"); return JE_ERR_FORMAT; }

    size_t max_bytes = (size_t)(e - b) / 2u + 1u;
    uint8_t *raw = (uint8_t *)malloc(max_bytes ? max_bytes : 1u);
    if (!raw) { je_file_buffer_free(&in); return JE_ERR_MEMORY; }
    size_t outn = 0;
    int high = -1;
    for (p = b; p < e; ++p) {
        unsigned char ch = (unsigned char)*p;
        int v = hexval(ch);
        if (isspace(ch)) continue;
        if (v < 0) {
            free(raw); je_file_buffer_free(&in); je_set_error("invalid character in PE_RAW_HEX"); return JE_ERR_FORMAT;
        }
        if (high < 0) high = v;
        else {
            raw[outn++] = (uint8_t)((high << 4) | v);
            high = -1;
        }
    }
    if (high >= 0 || outn < 64) {
        free(raw); je_file_buffer_free(&in); je_set_error("invalid PE_RAW_HEX payload"); return JE_ERR_FORMAT;
    }

    JEPEImage pe;
    st = je_pe_parse(raw, outn, &pe);
    if (st != JE_OK) { free(raw); je_file_buffer_free(&in); return st; }
    st = je_file_write_all(pe_path, raw, outn);
    free(raw);
    je_file_buffer_free(&in);
    return st;
}
