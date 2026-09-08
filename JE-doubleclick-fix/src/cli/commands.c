#include "je/je_loader.h"
#include "je/je_convert.h"
#include "je/je_error.h"
#include "je/je_log.h"
#include "je/je_process.h"
#include "je/je_cli.h"
#include <stdio.h>
#include <string.h>

static void print_usage(void){
    puts("JE CLI\n  je detect <file>\n  je run <file>\n  je launch <file>\n  je pe2je <in.exe> <out.jxj>\n  je je2pe <in.jxj> <out.exe>\n  je validate <file>");
}

static int usage(void){
    print_usage();
    return 2;
}

#ifdef _WIN32
static void interactive_help(void){
    print_usage();
    puts("\nNo command was specified.");
    puts("Press Enter to exit...");
    fflush(stdout);
    (void)getchar();
}
#endif

int je_cli_main(int argc, char **argv){
    if(argc < 2){
#ifdef _WIN32
        interactive_help();
        return 0;
#else
        return usage();
#endif
    }
    if(argc < 3) return usage();
    if(strcmp(argv[1],"detect")==0){
        JEDetectResult r=je_detect_file(argv[2]);
        printf("%d\n",(int)r); return 0;
    }
    if(strcmp(argv[1],"launch")==0){
        int code = 0;
        JEStatus s = je_dispatch_run_code(argv[2], &code);
        if(s != JE_OK){je_log_error(je_last_error()); return (int)s;}
        return code;
    }
    if(strcmp(argv[1],"run")==0){
        int code = 0;
        JEStatus s = je_load_and_run_code(argv[2], &code);
        if(s != JE_OK){je_log_error(je_last_error()); return (int)s;}
        return code;
    }
    if(strcmp(argv[1],"pe2je")==0 && argc>=4) return je_pe_to_je(argv[2],argv[3]);
    if(strcmp(argv[1],"je2pe")==0 && argc>=4) return je_je_to_pe(argv[2],argv[3]);
    if(strcmp(argv[1],"validate")==0){
        JEDetectResult r=je_detect_file(argv[2]);
        if(r==JE_DETECT_NO){je_log_error("not a valid JE file"); return (int)JE_ERR_FORMAT;}
        puts("valid JE"); return 0;
    }
    return usage();
}
