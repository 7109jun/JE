#ifndef JE_CONVERT_H
#define JE_CONVERT_H
#include "je.h"
#ifdef __cplusplus
extern "C" {
#endif
JEStatus je_pe_to_je(const char *pe_path, const char *je_path);
JEStatus je_je_to_pe(const char *je_path, const char *pe_path);
JEStatus je_roundtrip_check(const char *pe_path, const char *temp_je_path, const char *output_pe_path);
#ifdef __cplusplus
}
#endif
#endif
