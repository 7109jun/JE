#include "je/je_convert.h"
#include "je/je_file.h"
#include <stdlib.h>
#include <string.h>

JEStatus je_roundtrip_check(const char *pe_path, const char *temp_je_path, const char *output_pe_path) {
    JEStatus st = je_pe_to_je(pe_path, temp_je_path);
    if (st != JE_OK) return st;
    return je_je_to_pe(temp_je_path, output_pe_path);
}
