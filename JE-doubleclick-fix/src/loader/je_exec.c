#include "je/je_exec.h"
#include "je/je_error.h"
#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define JE_EXEC_MAX_LINES 4096u
#define JE_EXEC_MAX_LABELS 512u
#define JE_EXEC_LINE_MAX 1024u

typedef struct JELabel { char name[64]; size_t line; } JELabel;
typedef struct JEProgram { char **lines; size_t count; JELabel labels[JE_EXEC_MAX_LABELS]; size_t label_count; } JEProgram;

static char *trim(char *s) {
    while (*s && isspace((unsigned char)*s)) ++s;
    char *e = s + strlen(s);
    while (e > s && isspace((unsigned char)e[-1])) --e;
    *e = '\0';
    return s;
}

static int ident_char(int c) { return isalnum((unsigned char)c) || c == '_' || c == '.'; }

static int parse_reg(const char *s) {
    if (!s || (s[0] != 'R' && s[0] != 'r') || s[2] != '\0') return -1;
    if (s[1] < '0' || s[1] > '7') return -1;
    return s[1] - '0';
}

static int parse_u64(const char *s, uint64_t *out) {
    if (!s || !*s || !out) return 0;
    char *e = NULL;
    int base = 10;
    if (s[0] == '0' && (s[1] == 'x' || s[1] == 'X')) base = 16;
    errno = 0;
    unsigned long long v = strtoull(s, &e, base);
    if (e == s || *trim(e) != '\0' || errno == ERANGE) return 0;
    *out = (uint64_t)v;
    return 1;
}

static int parse_two_args(char *p, char **a, char **b) {
    char *comma = strchr(p, ',');
    if (!comma) return 0;
    *comma = '\0';
    *a = trim(p);
    *b = trim(comma + 1);
    return **a != '\0' && **b != '\0';
}

static void free_program(JEProgram *p) {
    if (!p) return;
    for (size_t i = 0; i < p->count; ++i) free(p->lines[i]);
    free(p->lines);
    memset(p, 0, sizeof(*p));
}

static JEStatus build_program(const char *code, size_t size, JEProgram *out) {
    memset(out, 0, sizeof(*out));
    size_t cap = 64;
    out->lines = (char **)calloc(cap, sizeof(char *));
    if (!out->lines) return JE_ERR_MEMORY;

    size_t pos = 0;
    while (pos < size) {
        size_t start = pos;
        while (pos < size && code[pos] != '\n') ++pos;
        size_t len = pos - start;
        if (pos < size) ++pos;
        while (len && code[start + len - 1] == '\r') --len;
        if (len >= JE_EXEC_LINE_MAX) { free_program(out); je_set_error("JE code line too long"); return JE_ERR_PARSE; }
        char *line = (char *)malloc(len + 1);
        if (!line) { free_program(out); return JE_ERR_MEMORY; }
        memcpy(line, code + start, len); line[len] = '\0';
        char *t = trim(line);
        if (*t == '#') { free(line); continue; }
        int in_string = 0, escaped = 0;
        for (char *q = t; *q; ++q) {
            if (in_string) {
                if (escaped) escaped = 0;
                else if (*q == '\\') escaped = 1;
                else if (*q == '"') in_string = 0;
            } else if (*q == '"') {
                in_string = 1;
            } else if (*q == '#') {
                *q = '\0';
                while (q > t && isspace((unsigned char)q[-1])) --q;
                *q = '\0';
                break;
            }
        }
        t = trim(t);
        if (*t == '\0') { free(line); continue; }

        char *colon = strchr(t, ':');
        if (colon) {
            int valid = colon > t;
            for (char *q = t; valid && q < colon; ++q) valid = ident_char((unsigned char)*q);
            if (valid) {
                if (out->label_count >= JE_EXEC_MAX_LABELS) { free(line); free_program(out); je_set_error("too many JE labels"); return JE_ERR_PARSE; }
                size_t ln = (size_t)(colon - t);
                if (ln >= sizeof(out->labels[0].name)) { free(line); free_program(out); return JE_ERR_PARSE; }
                for (size_t li = 0; li < out->label_count; ++li) {
                    if (strlen(out->labels[li].name) == ln && memcmp(out->labels[li].name, t, ln) == 0) {
                        free(line); free_program(out); je_set_error("duplicate JE label"); return JE_ERR_PARSE;
                    }
                }
                memcpy(out->labels[out->label_count].name, t, ln);
                out->labels[out->label_count].name[ln] = '\0';
                out->labels[out->label_count].line = out->count;
                ++out->label_count;
                t = trim(colon + 1);
                if (*t == '\0') { free(line); continue; }
            }
        }

        if (out->count == cap) {
            cap *= 2;
            char **nl = (char **)realloc(out->lines, cap * sizeof(char *));
            if (!nl) { free(line); free_program(out); return JE_ERR_MEMORY; }
            out->lines = nl;
        }
        if (t != line) memmove(line, t, strlen(t) + 1);
        out->lines[out->count++] = line;
        if (out->count > JE_EXEC_MAX_LINES) { free_program(out); je_set_error("too many JE code lines"); return JE_ERR_PARSE; }
    }
    return JE_OK;
}

static size_t find_label(const JEProgram *p, const char *name) {
    for (size_t i = 0; i < p->label_count; ++i)
        if (strcmp(p->labels[i].name, name) == 0) return p->labels[i].line;
    return SIZE_MAX;
}

static int next_token(char **p, char *buf, size_t cap) {
    char *s = *p;
    while (*s && isspace((unsigned char)*s)) ++s;
    if (!*s) { *p = s; return 0; }
    size_t n = 0;
    while (s[n] && !isspace((unsigned char)s[n]) && s[n] != ',') ++n;
    if (n + 1 > cap) return 0;
    memcpy(buf, s, n); buf[n] = '\0';
    *p = s + n;
    return 1;
}

void je_exec_init(JEExecContext *ctx) {
    if (!ctx) return;
    memset(ctx, 0, sizeof(*ctx));
    ctx->max_steps = 1000000u;
}


static int parse_value(JEExecContext *ctx, const char *s, uint64_t *value) {
    int r = parse_reg(s);
    if (r >= 0) { *value = ctx->regs[r]; return 1; }
    return parse_u64(s, value);
}

static JEStatus exec_line(JEExecContext *ctx, JEProgram *p, char *line, int *ip_changed) {
    if (ip_changed) *ip_changed = 0;
    char *s = trim(line);
    if (!*s) return JE_OK;
    char op[32];
    char *cursor = s;
    if (!next_token(&cursor, op, sizeof(op))) return JE_OK;
    for (char *q = op; *q; ++q) *q = (char)toupper((unsigned char)*q);

    if (strcmp(op, "NOP") == 0) return JE_OK;
    if (strcmp(op, "RET") == 0) {
        if (ctx->call_sp == 0) { ctx->halted = 1; return JE_OK; }
        ctx->ip = ctx->call_stack[--ctx->call_sp];
        if (ip_changed) *ip_changed = 1;
        return JE_OK;
    }
    if (strcmp(op, "EXIT") == 0) {
        char arg[64];
        cursor = trim(cursor);
        if (*cursor && !next_token(&cursor, arg, sizeof(arg))) return JE_ERR_PARSE;
        uint64_t v = 0;
        if (*cursor || (arg[0] && !parse_u64(arg, &v))) { je_set_error("invalid EXIT value"); return JE_ERR_PARSE; }
        ctx->exit_code = (int)(v & 0xFFu); ctx->halted = 1; return JE_OK;
    }
    if (strcmp(op, "MOV") == 0 || strcmp(op, "ADD") == 0 || strcmp(op, "SUB") == 0 || strcmp(op, "XOR") == 0 ||
        strcmp(op, "AND") == 0 || strcmp(op, "OR") == 0 || strcmp(op, "SHL") == 0 || strcmp(op, "SHR") == 0 || strcmp(op, "CMP") == 0) {
        char *a = NULL, *b = NULL; cursor = trim(cursor);
        if (!parse_two_args(cursor, &a, &b)) { je_set_error("two operands required"); return JE_ERR_PARSE; }
        int ra = parse_reg(a); if (ra < 0) { je_set_error("invalid JE register"); return JE_ERR_PARSE; }
        uint64_t value = 0; int rb = parse_reg(b);
        if (rb >= 0) value = ctx->regs[rb];
        else if (!parse_u64(b, &value)) { je_set_error("invalid JE operand"); return JE_ERR_PARSE; }
        if (strcmp(op, "MOV") == 0) {
            ctx->regs[ra] = value;
        } else if (strcmp(op, "ADD") == 0) {
            ctx->regs[ra] += value;
            ctx->zero = (ctx->regs[ra] == 0);
        } else if (strcmp(op, "SUB") == 0) {
            ctx->regs[ra] -= value;
            ctx->zero = (ctx->regs[ra] == 0);
        } else if (strcmp(op, "XOR") == 0) {
            ctx->regs[ra] ^= value;
            ctx->zero = (ctx->regs[ra] == 0);
        } else if (strcmp(op, "AND") == 0) {
            ctx->regs[ra] &= value;
            ctx->zero = (ctx->regs[ra] == 0);
        } else if (strcmp(op, "OR") == 0) {
            ctx->regs[ra] |= value;
            ctx->zero = (ctx->regs[ra] == 0);
        } else if (strcmp(op, "SHL") == 0) {
            ctx->regs[ra] <<= (value & 63u);
            ctx->zero = (ctx->regs[ra] == 0);
        } else if (strcmp(op, "SHR") == 0) {
            ctx->regs[ra] >>= (value & 63u);
            ctx->zero = (ctx->regs[ra] == 0);
        } else {
            ctx->last_cmp = (int64_t)(ctx->regs[ra] - value);
            ctx->zero = (ctx->regs[ra] == value);
        }
        return JE_OK;
    }
    if (strcmp(op, "INC") == 0 || strcmp(op, "DEC") == 0) {
        int r = parse_reg(trim(cursor));
        if (r < 0) { je_set_error("INC/DEC expects register"); return JE_ERR_PARSE; }
        if (strcmp(op, "INC") == 0) ++ctx->regs[r]; else --ctx->regs[r];
        ctx->zero = (ctx->regs[r] == 0);
        return JE_OK;
    }
    if (strcmp(op, "PUSH") == 0) {
        uint64_t v = 0;
        if (ctx->sp >= 256 || !parse_value(ctx, trim(cursor), &v)) {
            je_set_error("invalid PUSH"); return JE_ERR_PARSE;
        }
        ctx->stack[ctx->sp++] = v;
        return JE_OK;
    }
    if (strcmp(op, "POP") == 0) {
        int r = parse_reg(trim(cursor));
        if (r < 0 || ctx->sp == 0) { je_set_error("invalid POP"); return JE_ERR_PARSE; }
        ctx->regs[r] = ctx->stack[--ctx->sp];
        return JE_OK;
    }
    if (strcmp(op, "CALL") == 0) {
        char *label = trim(cursor);
        size_t target = find_label(p, label);
        if (target == SIZE_MAX || ctx->call_sp >= 256) {
            je_set_error("invalid CALL"); return JE_ERR_PARSE;
        }
        ctx->call_stack[ctx->call_sp++] = (size_t)ctx->ip + 1;
        ctx->ip = target;
        if (ip_changed) *ip_changed = 1;
        return JE_OK;
    }
    if (strcmp(op, "PRINT") == 0) {
        char *arg = trim(cursor);
        if (*arg == '"') {
            size_t n = strlen(arg);
            if (n < 2 || arg[n - 1] != '"') { je_set_error("unterminated PRINT string"); return JE_ERR_PARSE; }
            arg[n - 1] = '\0';
            fputs(arg + 1, stdout); fputc('\n', stdout);
            return JE_OK;
        }
        int r = parse_reg(arg);
        if (r < 0) { je_set_error("PRINT expects register or string"); return JE_ERR_PARSE; }
        printf("%llu\n", (unsigned long long)ctx->regs[r]);
        return JE_OK;
    }
    if (strcmp(op, "JMP") == 0 || strcmp(op, "JZ") == 0 || strcmp(op, "JNZ") == 0) {
        char *label = trim(cursor);
        size_t target = find_label(p, label);
        if (target == SIZE_MAX) { je_set_error("unknown JE label"); return JE_ERR_PARSE; }
        int take = strcmp(op, "JMP") == 0 || (strcmp(op, "JZ") == 0 ? ctx->zero : !ctx->zero);
        if (take) {
            ctx->ip = target;
            if (ip_changed) *ip_changed = 1;
        }
        return JE_OK;
    }
    je_set_error("unsupported JE instruction");
    return JE_ERR_UNSUPPORTED;
}

JEStatus je_exec_text(JEExecContext *ctx, const char *code, size_t size) {
    if (!ctx || !code) return JE_ERR_INVALID_ARGUMENT;
    JEProgram p;
    JEStatus st = build_program(code, size, &p);
    if (st != JE_OK) return st;
    ctx->ip = 0; ctx->halted = 0; ctx->exit_code = 0; ctx->steps = 0;
    while (!ctx->halted && ctx->ip < p.count) {
        if (++ctx->steps > ctx->max_steps) { free_program(&p); je_set_error("JE execution step limit exceeded"); return JE_ERR_UNSUPPORTED; }
        int ip_changed = 0;
        st = exec_line(ctx, &p, p.lines[ctx->ip], &ip_changed);
        if (st != JE_OK) { free_program(&p); return st; }
        if (!ctx->halted && !ip_changed) ++ctx->ip;
    }
    free_program(&p);
    return JE_OK;
}
