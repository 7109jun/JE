#include "je/je_error.h"
#include <stdio.h>
#include <string.h>

static char g_error[512];

const char *je_last_error(void) { return g_error; }

void je_set_error(const char *message) {
    if (!message) message = "unknown error";
    snprintf(g_error, sizeof(g_error), "%s", message);
}
