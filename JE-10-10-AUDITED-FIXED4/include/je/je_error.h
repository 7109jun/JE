#ifndef JE_ERROR_H
#define JE_ERROR_H
#include "je.h"
#ifdef __cplusplus
extern "C" {
#endif
const char *je_last_error(void);
void je_set_error(const char *message);
#ifdef __cplusplus
}
#endif
#endif
