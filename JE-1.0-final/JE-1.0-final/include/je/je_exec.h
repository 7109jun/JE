#ifndef JE_EXEC_H
#define JE_EXEC_H

#include "je.h"
#include <stdint.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct JEExecContext {
    uint64_t regs[8];
    uint64_t ip;
    int64_t zero;
    int64_t last_cmp;
    int halted;
    int exit_code;
    uint64_t steps;
    uint64_t max_steps;
    uint64_t stack[256];
    size_t sp;
    size_t call_stack[256];
    size_t call_sp;
} JEExecContext;

void je_exec_init(JEExecContext *ctx);
JEStatus je_exec_text(JEExecContext *ctx, const char *code, size_t size);

#ifdef __cplusplus
}
#endif
#endif
