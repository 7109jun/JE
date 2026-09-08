#include "je/je_pe.h"
#include "je/je_file.h"

JEStatus je_pe_write(const JEPEImage *image, const char *path) {
    if (!image || !path || !*path) return JE_ERR_INVALID_ARGUMENT;
    if (!image->data || image->size == 0) return JE_ERR_FORMAT;
    /* The parsed PE image retains its original complete byte representation. */
    return je_file_write_all(path, image->data, image->size);
}
