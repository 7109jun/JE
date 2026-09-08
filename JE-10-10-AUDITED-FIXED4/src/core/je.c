#include "je/je.h"
#include <string.h>

const char *je_status_string(JEStatus status) {
    switch (status) {
        case JE_OK: return "ok";
        case JE_ERR_INVALID_ARGUMENT: return "invalid argument";
        case JE_ERR_IO: return "I/O error";
        case JE_ERR_PARSE: return "parse error";
        case JE_ERR_FORMAT: return "format error";
        case JE_ERR_UNSUPPORTED: return "unsupported";
        case JE_ERR_MEMORY: return "memory error";
        case JE_ERR_INTERNAL: return "internal error";
        default: return "unknown error";
    }
}

int je_has_magic(const char *data, size_t size) {
    if (!data || size < JE_MAGIC_LEN) return 0;
    return memcmp(data, JE_MAGIC, JE_MAGIC_LEN) == 0;
}
