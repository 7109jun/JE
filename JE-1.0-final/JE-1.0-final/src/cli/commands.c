#include "je/je_loader.h"
#include "je/je_convert.h"
#include "je/je_error.h"
#include "je/je_log.h"
#include "je/je_process.h"
#include "je/je_cli.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>

static void print_usage(void) {
    puts("JE CLI");
    puts("  je <file.jxj>                 Run a JE file");
    puts("  je detect <file>              Detect JE/PE format");
    puts("  je run <file.jxj>             Run JE text code");
    puts("  je launch <file>              Run JE or native PE");
    puts("  je pe2je <in.exe> <out.jxj>   Convert PE to JE");
    puts("  je je2pe <in.jxj> <out.exe>   Convert JE to PE");
    puts("  je validate <file>            Validate JE header and CODE");
}

static int usage(void) {
    print_usage();
    return 2;
}

#ifdef _WIN32
static void wait_for_enter(void) {
    char line[8];
    puts("\nPress Enter to close...");
    fflush(stdout);
    (void)fgets(line, sizeof(line), stdin);
}
#endif

static int run_file(const char *path, int dispatch) {
    int code = 0;
    JEStatus s = dispatch ? je_dispatch_run_code(path, &code)
                          : je_load_and_run_code(path, &code);
    if (s != JE_OK) {
        const char *msg = je_last_error();
        if (msg && *msg) je_log_error(msg);
        else je_log_error("JE execution failed");
        return (int)s;
    }
    return code;
}

int je_cli_main(int argc, char **argv) {
    /* Explorer / shell integration: `je file.jxj` executes it directly. */
    if (argc == 2) {
        const char *path = argv[1];
        if (path && *path) return run_file(path, 1);
#ifdef _WIN32
        wait_for_enter();
        return 0;
#else
        return usage();
#endif
    }

    if (argc < 2) {
#ifdef _WIN32
        print_usage();
        puts("\nNo command was specified.");
        wait_for_enter();
        return 0;
#else
        return usage();
#endif
    }

    if (strcmp(argv[1], "detect") == 0 && argc >= 3) {
        JEDetectResult r = je_detect_file(argv[2]);
        printf("%d\n", (int)r);
        return 0;
    }

    if (strcmp(argv[1], "launch") == 0 && argc >= 3)
        return run_file(argv[2], 1);

    if (strcmp(argv[1], "run") == 0 && argc >= 3)
        return run_file(argv[2], 0);

    if (strcmp(argv[1], "pe2je") == 0 && argc >= 4)
        return je_pe_to_je(argv[2], argv[3]);

    if (strcmp(argv[1], "je2pe") == 0 && argc >= 4)
        return je_je_to_pe(argv[2], argv[3]);

    if (strcmp(argv[1], "validate") == 0 && argc >= 3) {
        /* Validation is intentionally non-executing: loading is performed by
           checking header + CODE structure, while execution is not entered. */
        JEDetectResult r = je_detect_file(argv[2]);
        if (r == JE_DETECT_NO) {
            je_log_error("not a valid JE file");
            return (int)JE_ERR_FORMAT;
        }
        puts("valid JE");
        return 0;
    }

    return usage();
}
