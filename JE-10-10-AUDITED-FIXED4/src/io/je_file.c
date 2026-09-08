#include "je/je_file.h"
#include "je/je_memory.h"
#include "je/je_error.h"
#include <stdio.h>

JEStatus je_file_read_all(const char *path, JEFileBuffer *out) {
    FILE *f; long end; size_t size;
    if (!path || !out) return JE_ERR_INVALID_ARGUMENT;
    out->data = NULL; out->size = 0;
    f = fopen(path, "rb"); if (!f) { je_set_error("cannot open file"); return JE_ERR_IO; }
    if (fseek(f, 0, SEEK_END) != 0) { fclose(f); return JE_ERR_IO; }
    end = ftell(f); if (end < 0) { fclose(f); return JE_ERR_IO; }
    if (fseek(f, 0, SEEK_SET) != 0) { fclose(f); return JE_ERR_IO; }
    size = (size_t)end;
    out->data = (uint8_t *)je_malloc(size ? size : 1);
    if (!out->data) { fclose(f); return JE_ERR_MEMORY; }
    if (size && fread(out->data, 1, size, f) != size) { je_free(out->data); out->data=NULL; fclose(f); return JE_ERR_IO; }
    fclose(f); out->size=size; return JE_OK;
}

void je_file_buffer_free(JEFileBuffer *buffer) {
    if (!buffer) return;
    je_free(buffer->data);
    buffer->data = NULL;
    buffer->size = 0;
}

JEStatus je_file_write_all(const char *path, const void *data, size_t size) {
    FILE *f;
    if (!path || (!data && size)) return JE_ERR_INVALID_ARGUMENT;
    f=fopen(path,"wb"); if(!f) return JE_ERR_IO;
    if(size && fwrite(data,1,size,f)!=size){fclose(f);return JE_ERR_IO;}
    if(fclose(f)!=0) return JE_ERR_IO;
    return JE_OK;
}
