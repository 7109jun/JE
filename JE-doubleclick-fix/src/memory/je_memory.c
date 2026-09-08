#include "je/je_memory.h"
#include <stdlib.h>
void *je_malloc(size_t size) { return malloc(size); }
void *je_calloc(size_t count, size_t size) { return calloc(count, size); }
void je_free(void *ptr) { free(ptr); }
