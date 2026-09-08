#ifndef JE_LOADER_H
#define JE_LOADER_H
#include "je.h"
#ifdef __cplusplus
extern "C" {
#endif
typedef enum JEDetectResult { JE_DETECT_NO = 0, JE_DETECT_JXJ = 1, JE_DETECT_MAGIC = 2 } JEDetectResult;
JEDetectResult je_detect_file(const char *path);
JEStatus je_load_and_run(const char *path);
JEStatus je_load_and_run_code(const char *path, int *exit_code);
JEStatus je_dispatch_run_code(const char *path, int *exit_code);
JEStatus je_dispatch_run(const char *path);
#ifdef __cplusplus
}
#endif
#endif
