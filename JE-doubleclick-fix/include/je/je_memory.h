#ifndef JE_MEMORY_H
#define JE_MEMORY_H
#include "je.h"
#ifdef __cplusplus
extern "C" {
#endif
void *je_malloc(size_t size);
void *je_calloc(size_t count, size_t size);
void je_free(void *ptr);
#ifdef __cplusplus
}
#endif
#endif
