#ifndef JE_TEXT_H
#define JE_TEXT_H

#include "je.h"
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum JETokenType {
    JE_TOKEN_EOF = 0,
    JE_TOKEN_IDENTIFIER,
    JE_TOKEN_STRING,
    JE_TOKEN_NUMBER,
    JE_TOKEN_LBRACE,
    JE_TOKEN_RBRACE,
    JE_TOKEN_LBRACKET,
    JE_TOKEN_RBRACKET,
    JE_TOKEN_EQUAL,
    JE_TOKEN_COLON,
    JE_TOKEN_NEWLINE,
    JE_TOKEN_COMMA
} JETokenType;

typedef struct JEToken {
    JETokenType type;
    const char *start;
    size_t length;
    size_t line;
    size_t column;
    uint64_t number;
} JEToken;

typedef struct JETokenizer {
    const char *data;
    size_t size;
    size_t pos;
    size_t line;
    size_t column;
} JETokenizer;

typedef enum JEHeaderFlag {
    JE_HEADER_NONE = 0,
    JE_HEADER_HAS_MAGIC = 1u << 0
} JEHeaderFlag;

typedef struct JEHeader {
    char author[128];
    char format[32];
    uint32_t version;
    uint16_t architecture;
    uint32_t flags;
} JEHeader;

#define JE_ARCH_UNKNOWN 0u
#define JE_ARCH_X86 0x014Cu
#define JE_ARCH_X86_64 0x8664u
#define JE_ARCH_ARM 0x01C0u
#define JE_ARCH_ARM64 0xAA64u

JEStatus je_text_validate(const char *text, size_t size);
void je_tokenizer_init(JETokenizer *tz, const char *data, size_t size);
JEStatus je_token_next(JETokenizer *tz, JEToken *out);
JEStatus je_parse_header(const char *text, size_t size, JEHeader *out, size_t *body_offset);
int je_has_jxj_extension(const char *path);

#ifdef __cplusplus
}
#endif

#endif
