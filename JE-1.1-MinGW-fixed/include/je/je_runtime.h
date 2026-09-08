#ifndef JE_RUNTIME_H
#define JE_RUNTIME_H

#include "je.h"
#include "je_exec.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Host-linked C runtime/file I/O ABI for JE IMPORT calls. */
JEStatus je_runtime_call(JEExecContext *ctx, const char *name);
void je_runtime_cleanup(JEExecContext *ctx);

#ifdef __cplusplus
}
#endif
#endif
