#ifndef JE_JE_H
#define JE_JE_H

#include <stddef.h>
#include <stdint.h>

#define JE_VERSION_MAJOR 1
#define JE_VERSION_MINOR 1
#define JE_MAGIC "[ JE : O]"
#define JE_MAGIC_LEN 9u
#define JE_DEFAULT_EXTENSION ".jxj"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum JEStatus {
    JE_OK = 0,
    JE_ERR_INVALID_ARGUMENT = 1,
    JE_ERR_IO = 2,
    JE_ERR_PARSE = 3,
    JE_ERR_FORMAT = 4,
    JE_ERR_UNSUPPORTED = 5,
    JE_ERR_MEMORY = 6,
    JE_ERR_INTERNAL = 7
} JEStatus;

const char *je_status_string(JEStatus status);
int je_has_magic(const char *data, size_t size);

#ifdef __cplusplus
}
#endif

#endif
