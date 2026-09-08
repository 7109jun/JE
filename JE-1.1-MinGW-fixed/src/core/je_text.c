#include "je/je_text.h"
#include <ctype.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>

static int is_ident_start(unsigned char c) {
    return isalpha(c) || c == '_' || c == '.';
}

static int is_ident_char(unsigned char c) {
    return isalnum(c) || c == '_' || c == '.' || c == '-';
}

static void advance_char(JETokenizer *tz) {
    if (tz->pos >= tz->size) return;
    if (tz->data[tz->pos] == '\n') {
        tz->line++;
        tz->column = 1;
    } else {
        tz->column++;
    }
    tz->pos++;
}

void je_tokenizer_init(JETokenizer *tz, const char *data, size_t size) {
    if (!tz) return;
    tz->data = data;
    tz->size = size;
    tz->pos = 0;
    tz->line = 1;
    tz->column = 1;
}

static JEStatus token_simple(JETokenizer *tz, JEToken *out, JETokenType type) {
    out->type = type;
    out->start = tz->data + tz->pos;
    out->length = 1;
    out->line = tz->line;
    out->column = tz->column;
    out->number = 0;
    advance_char(tz);
    return JE_OK;
}

JEStatus je_token_next(JETokenizer *tz, JEToken *out) {
    if (!tz || !out || (!tz->data && tz->size)) return JE_ERR_INVALID_ARGUMENT;
    memset(out, 0, sizeof(*out));

    for (;;) {
        if (tz->pos >= tz->size) {
            out->type = JE_TOKEN_EOF;
            out->start = tz->data ? tz->data + tz->pos : NULL;
            out->line = tz->line;
            out->column = tz->column;
            return JE_OK;
        }

        unsigned char c = (unsigned char)tz->data[tz->pos];
        if (c == ' ' || c == '\t' || c == '\r') {
            advance_char(tz);
            continue;
        }
        if (c == '#') {
            while (tz->pos < tz->size && tz->data[tz->pos] != '\n') advance_char(tz);
            continue;
        }
        if (c == '\n') return token_simple(tz, out, JE_TOKEN_NEWLINE);
        break;
    }

    unsigned char c = (unsigned char)tz->data[tz->pos];
    if (c == '{') return token_simple(tz, out, JE_TOKEN_LBRACE);
    if (c == '}') return token_simple(tz, out, JE_TOKEN_RBRACE);
    if (c == '[') return token_simple(tz, out, JE_TOKEN_LBRACKET);
    if (c == ']') return token_simple(tz, out, JE_TOKEN_RBRACKET);
    if (c == '=') return token_simple(tz, out, JE_TOKEN_EQUAL);
    if (c == ':') return token_simple(tz, out, JE_TOKEN_COLON);
    if (c == ',') return token_simple(tz, out, JE_TOKEN_COMMA);

    out->start = tz->data + tz->pos;
    out->line = tz->line;
    out->column = tz->column;

    if (c == '"') {
        advance_char(tz);
        const size_t begin = tz->pos;
        int escaped = 0;
        while (tz->pos < tz->size) {
            unsigned char q = (unsigned char)tz->data[tz->pos];
            if (q == '\n' || q == '\r' || q == 0) return JE_ERR_PARSE;
            if (!escaped && q == '"') {
                out->type = JE_TOKEN_STRING;
                out->start = tz->data + begin;
                out->length = tz->pos - begin;
                advance_char(tz);
                return JE_OK;
            }
            if (!escaped && q == '\\') escaped = 1;
            else escaped = 0;
            advance_char(tz);
        }
        return JE_ERR_PARSE;
    }

    if (isdigit(c)) {
        size_t begin = tz->pos;
        int base = 10;
        if (c == '0' && tz->pos + 1 < tz->size &&
            (tz->data[tz->pos + 1] == 'x' || tz->data[tz->pos + 1] == 'X')) {
            base = 16;
            advance_char(tz);
            advance_char(tz);
            begin = tz->pos;
        }
        char buf[32];
        size_t n = 0;
        int too_long = 0;
        while (tz->pos < tz->size) {
            unsigned char d = (unsigned char)tz->data[tz->pos];
            int valid = base == 16 ? isxdigit(d) : isdigit(d);
            if (!valid) break;
            if (n + 1 < sizeof(buf)) buf[n++] = (char)d;
            else too_long = 1;
            advance_char(tz);
        }
        buf[n] = '\0';
        if (too_long) return JE_ERR_PARSE;
        char *endp = NULL;
        errno = 0;
        unsigned long long v = strtoull(buf, &endp, base);
        if (endp == buf || *endp != '\0' || errno == ERANGE) return JE_ERR_PARSE;
        out->type = JE_TOKEN_NUMBER;
        out->start = tz->data + begin;
        out->length = n;
        out->number = (uint64_t)v;
        return JE_OK;
    }

    if (is_ident_start(c)) {
        size_t begin = tz->pos;
        while (tz->pos < tz->size && is_ident_char((unsigned char)tz->data[tz->pos])) advance_char(tz);
        out->type = JE_TOKEN_IDENTIFIER;
        out->start = tz->data + begin;
        out->length = tz->pos - begin;
        return JE_OK;
    }

    return JE_ERR_PARSE;
}

JEStatus je_text_validate(const char *text, size_t size) {
    if (!text) return JE_ERR_INVALID_ARGUMENT;
    if (size == 0) return JE_ERR_FORMAT;
    for (size_t i = 0; i < size; ++i) {
        unsigned char c = (unsigned char)text[i];
        if (c == 0) return JE_ERR_FORMAT;
        if (!(c == '\n' || c == '\r' || c == '\t' || c >= 0x20)) return JE_ERR_FORMAT;
    }
    return JE_OK;
}

static void copy_token_string(char *dst, size_t dst_size, const JEToken *tok) {
    if (!dst || dst_size == 0 || !tok) return;
    size_t n = tok->length < dst_size - 1 ? tok->length : dst_size - 1;
    memcpy(dst, tok->start, n);
    dst[n] = '\0';
}

static int token_equals(const JEToken *tok, const char *s) {
    size_t n = strlen(s);
    return tok->length == n && memcmp(tok->start, s, n) == 0;
}

static JEStatus skip_newlines(JETokenizer *tz, JEToken *tok) {
    do {
        JEStatus st = je_token_next(tz, tok);
        if (st != JE_OK) return st;
    } while (tok->type == JE_TOKEN_NEWLINE);
    return JE_OK;
}

JEStatus je_parse_header(const char *text, size_t size, JEHeader *out, size_t *body_offset) {
    if (!text || !out) return JE_ERR_INVALID_ARGUMENT;
    if (je_text_validate(text, size) != JE_OK) return JE_ERR_FORMAT;
    memset(out, 0, sizeof(*out));
    out->format[0] = '\0';
    if (!je_has_magic(text, size)) return JE_ERR_FORMAT;
    out->flags |= JE_HEADER_HAS_MAGIC;

    size_t pos = JE_MAGIC_LEN;
    while (pos < size && (text[pos] == '\r' || text[pos] == '\n' || text[pos] == ' ' || text[pos] == '\t')) ++pos;
    JETokenizer tz;
    JEToken tok;
    je_tokenizer_init(&tz, text + pos, size - pos);

    for (;;) {
        JEStatus st = skip_newlines(&tz, &tok);
        if (st != JE_OK) return st;
        if (tok.type == JE_TOKEN_EOF || token_equals(&tok, "SYSTEM") || token_equals(&tok, "SECTION") || token_equals(&tok, "CODE")) {
            if (body_offset) *body_offset = pos + (size_t)(tok.start - (text + pos));
            return JE_OK;
        }
        if (tok.type != JE_TOKEN_IDENTIFIER) return JE_ERR_PARSE;
        JEToken key = tok;
        st = je_token_next(&tz, &tok);
        if (st != JE_OK || tok.type != JE_TOKEN_EQUAL) return JE_ERR_PARSE;
        st = je_token_next(&tz, &tok);
        if (st != JE_OK) return st;

        if (token_equals(&key, "AUTHOR")) {
            if (tok.type != JE_TOKEN_STRING) return JE_ERR_PARSE;
            copy_token_string(out->author, sizeof(out->author), &tok);
        } else if (token_equals(&key, "FORMAT")) {
            if (tok.type != JE_TOKEN_STRING) return JE_ERR_PARSE;
            copy_token_string(out->format, sizeof(out->format), &tok);
        } else if (token_equals(&key, "VERSION")) {
            if (tok.type != JE_TOKEN_NUMBER || tok.number > UINT32_MAX) return JE_ERR_PARSE;
            out->version = (uint32_t)tok.number;
        } else if (token_equals(&key, "ARCH")) {
            if (tok.type != JE_TOKEN_IDENTIFIER) return JE_ERR_PARSE;
            if (token_equals(&tok, "X86_64")) out->architecture = JE_ARCH_X86_64;
            else if (token_equals(&tok, "X86")) out->architecture = JE_ARCH_X86;
            else if (token_equals(&tok, "ARM64")) out->architecture = JE_ARCH_ARM64;
            else if (token_equals(&tok, "ARM")) out->architecture = JE_ARCH_ARM;
            else {
                return JE_ERR_PARSE;
            }
        }
        do {
            st = je_token_next(&tz, &tok);
            if (st != JE_OK) return st;
        } while (tok.type != JE_TOKEN_NEWLINE && tok.type != JE_TOKEN_EOF);
        if (tok.type == JE_TOKEN_EOF) {
            if (body_offset) *body_offset = size;
            return JE_OK;
        }
    }
}

int je_has_jxj_extension(const char *path) {
    if (!path) return 0;
    size_t n = strlen(path);
    const char *ext = ".jxj";
    size_t en = 4;
    if (n < en) return 0;
    const char *p = path + n - en;
    for (size_t i = 0; i < en; ++i) {
        if ((char)tolower((unsigned char)p[i]) != ext[i]) return 0;
    }
    return 1;
}
