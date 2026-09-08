#ifndef JE_PROCESS_H
#define JE_PROCESS_H
#include "je.h"
#ifdef __cplusplus
extern "C" {
#endif
/* Launch an existing native Windows executable. On non-Windows platforms returns JE_ERR_UNSUPPORTED. */
JEStatus je_process_launch(const char *path, const char *const *argv, int *exit_code);
#ifdef __cplusplus
}
#endif
#endif
