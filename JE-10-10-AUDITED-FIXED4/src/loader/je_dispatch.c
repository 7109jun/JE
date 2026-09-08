#include "je/je_loader.h"
#include "je/je_process.h"
#include "je/je_file.h"
#include "je/je_pe.h"
#include "je/je_error.h"
#include <string.h>

JEStatus je_dispatch_run_code(const char *path, int *exit_code) {
    if (!path) return JE_ERR_INVALID_ARGUMENT;
    JEDetectResult d = je_detect_file(path);
    if (d == JE_DETECT_JXJ || d == JE_DETECT_MAGIC) {
        return je_load_and_run_code(path, exit_code);
    }
    JEFileBuffer b = {0};
    JEStatus st = je_file_read_all(path, &b);
    if (st != JE_OK) return st;
    JEPEImage pe;
    st = je_pe_parse(b.data, b.size, &pe);
    je_file_buffer_free(&b);
    if (st != JE_OK) { je_set_error("file is neither JE nor a supported PE executable"); return JE_ERR_FORMAT; }
    return je_process_launch(path, NULL, exit_code);
}

JEStatus je_dispatch_run(const char *path) {
    return je_dispatch_run_code(path, NULL);
}
