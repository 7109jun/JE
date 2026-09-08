#include "je/je_log.h"
#include <stdio.h>
void je_log_info(const char *message) { if (message) fprintf(stdout, "[JE] %s\n", message); }
void je_log_error(const char *message) { if (message) fprintf(stderr, "[JE:ERR] %s\n", message); }
