#ifndef JE_FILE_H
#define JE_FILE_H
#include "je.h"
#include <stdint.h>
#ifdef __cplusplus
extern "C" {
#endif
typedef struct JEFileBuffer { uint8_t *data; size_t size; } JEFileBuffer;
JEStatus je_file_read_all(const char *path, JEFileBuffer *out);
void je_file_buffer_free(JEFileBuffer *buffer);
JEStatus je_file_write_all(const char *path, const void *data, size_t size);
#ifdef __cplusplus
}
#endif
#endif
