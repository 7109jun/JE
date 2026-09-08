#include "je/je.h"
#include "je/je_text.h"
#include "je/je_pe.h"
#include "je/je_convert.h"
#include "je/je_file.h"
#include "je/je_exec.h"
#include "je/je_loader.h"
#include <assert.h>
#include <string.h>
#include <stdint.h>
#include <stdio.h>
#undef assert
#define assert(cond) do { if (!(cond)) { fprintf(stderr, "smoke assertion failed: %s\n", #cond); return 1; } } while (0)

static void put16(uint8_t *p,uint16_t v){p[0]=(uint8_t)v;p[1]=(uint8_t)(v>>8);}
static void put32(uint8_t *p,uint32_t v){p[0]=(uint8_t)v;p[1]=(uint8_t)(v>>8);p[2]=(uint8_t)(v>>16);p[3]=(uint8_t)(v>>24);}
static void build_fake_pe(uint8_t *b,size_t n){
    memset(b,0,n); put16(b,0x5A4D); put32(b+0x3c,0x80); put32(b+0x80,0x4550);
    put16(b+0x84,0x8664); put16(b+0x86,1); put16(b+0x94,0xF0);
    put16(b+0x98,0x20B); put32(b+0xA8,0x1000); /* ImageBase intentionally left zero in synthetic test */ put32(b+0xB8,0x1000); put32(b+0xBC,0x200); put32(b+0xD0,0x4000); put32(b+0xD4,0x200);
    put16(b+0xDC,3);
    memcpy(b+0x188,".text",5); put32(b+0x190,0x10); put32(b+0x194,0x1000); put32(b+0x198,0x200); put32(b+0x19C,0x200); put32(b+0x1A4,0x60000020);
    b[0x200]=0xC3;
}

int main(void){
    const char sample[]="[ JE : O]\nAUTHOR = \"7109jun\"\nFORMAT = \"JE\"\nVERSION = 1\nARCH = X86_64\n\nSYSTEM {\n ENTRY = main\n}\n";
    JEHeader h; size_t body=0; assert(je_has_magic(sample,strlen(sample))); assert(je_parse_header(sample,strlen(sample),&h,&body)==JE_OK); assert(strcmp(h.author,"7109jun")==0);
    JETokenizer tz; JEToken tok; je_tokenizer_init(&tz,"SYSTEM { ENTRY = main }",strlen("SYSTEM { ENTRY = main }")); assert(je_token_next(&tz,&tok)==JE_OK); assert(tok.type==JE_TOKEN_IDENTIFIER); assert(je_has_jxj_extension("HELLO.JXJ"));
    assert(je_has_jxj_extension("program.jxj"));

    uint8_t pe[0x400]; build_fake_pe(pe,sizeof(pe));
    JEPEImage img; assert(je_pe_parse(pe,sizeof(pe),&img)==JE_OK); assert(img.machine==JE_PE_MACHINE_AMD64); assert(img.section_count==1); assert(je_pe_find_section(&img,".text")!=NULL); uint32_t off=0; assert(je_pe_rva_to_offset(&img,0x1000,&off)&&off==0x200);
    {
        uint8_t badpe[0x200]; memset(badpe,0,sizeof(badpe));
        put16(badpe,0x5A4D); put32(badpe+0x3c,0x80); put32(badpe+0x80,0x4550);
        put16(badpe+0x84,0x8664); put16(badpe+0x86,1); put16(badpe+0x94,0x70);
        put16(badpe+0x98,0x20B); put32(badpe+0x80+24+108,16);
        assert(je_pe_parse(badpe,sizeof(badpe),&img)==JE_ERR_FORMAT);
    }

    const char *in="je_test_in.exe", *jx="je_test.jxj", *out="je_test_out.exe";
    assert(je_file_write_all(in,pe,sizeof(pe))==JE_OK);
    assert(je_pe_to_je(in,jx)==JE_OK);
    assert(je_je_to_pe(jx,out)==JE_OK);
    JEFileBuffer rb={0}; assert(je_file_read_all(out,&rb)==JE_OK); assert(rb.size==sizeof(pe)); assert(memcmp(rb.data,pe,sizeof(pe))==0); je_file_buffer_free(&rb);
    const char *code = "main:\n MOV R0, 40\n ADD R0, 2\n CMP R0, 42\n JZ done\n EXIT 9\ndone:\n PRINT R0\n EXIT 0\n";
    JEExecContext ctx; je_exec_init(&ctx);
    assert(je_exec_text(&ctx, code, strlen(code)) == JE_OK);
    assert(ctx.regs[0] == 42 && ctx.exit_code == 0);

    const char *advanced = "main:\n MOV R0, 4\n PUSH R0\n CALL twice\n POP R1\n PRINT R1\n EXIT 0\ntwice:\n SHL R0, 1\n RET\n";
    je_exec_init(&ctx);
    assert(je_exec_text(&ctx, advanced, strlen(advanced)) == JE_OK);
    assert(ctx.regs[1] == 4 && ctx.regs[0] == 8 && ctx.exit_code == 0);

    const char *runpath = "je_exec_test.jxj";
    FILE *rf = fopen(runpath, "wb"); assert(rf);
    fputs("[ JE : O]\nVERSION = 1\nFORMAT = \"JE\"\nARCH = X86_64\n\nCODE {\n PRINT \"smoke\"\n MOV R0, 7\n EXIT 0\n}\n", rf);
    fclose(rf);
    assert(je_load_and_run(runpath) == JE_OK);
    const char *bare = "je_extensionless";
    FILE *bf = fopen(bare, "wb"); assert(bf);
    fputs("[ JE : O]\nVERSION = 1\nFORMAT = \"JE\"\nARCH = X86_64\n\nCODE {\n EXIT 0\n}\n", bf);
    fclose(bf);
    assert(je_detect_file(bare) == JE_DETECT_MAGIC);
    assert(je_load_and_run(bare) == JE_OK);

    const char *flagcode = "MOV R0, 0\nCMP R0, 0\nADD R0, 1\nJZ bad\nMOV R1, 7\nJMP done\nbad:\nMOV R1, 9\ndone:\nEXIT 0\n";
    je_exec_init(&ctx);
    assert(je_exec_text(&ctx, flagcode, strlen(flagcode)) == JE_OK);
    assert(ctx.regs[1] == 7);

    const char *selfjmp = "start:\n INC R0\n JMP start\n";
    je_exec_init(&ctx);
    ctx.max_steps = 8;
    assert(je_exec_text(&ctx, selfjmp, strlen(selfjmp)) == JE_ERR_UNSUPPORTED);
    assert(ctx.regs[0] == 4 && ctx.steps == 9);

    const char *commentcode = "MOV R0, 7 # inline comment\nEXIT 0\n";
    je_exec_init(&ctx);
    assert(je_exec_text(&ctx, commentcode, strlen(commentcode)) == JE_OK);
    assert(ctx.regs[0] == 7);
    const char *duplabel = "main: MOV R0, 1\nmain: MOV R0, 2\nEXIT 0\n";
    je_exec_init(&ctx);
    assert(je_exec_text(&ctx, duplabel, strlen(duplabel)) == JE_ERR_PARSE);

    const char *nocode = "je_nocode.jxj";
    FILE *nf = fopen(nocode, "wb"); assert(nf);
    fputs("[ JE : O]\nVERSION = 1\nFORMAT = \"JE\"\nARCH = X86_64\nSYSTEM { NOTE = \"CODE { forged }\" }\n", nf);
    fclose(nf);
    assert(je_load_and_run(nocode) == JE_ERR_FORMAT);

    const char *badjxj = "je_bad.jxj";
    FILE *uj = fopen(badjxj, "wb"); assert(uj); fputs("not JE\n", uj); fclose(uj);
    assert(je_detect_file(badjxj) == JE_DETECT_NO);

    const char *badarch = "je_badarch.jxj";
    FILE *af = fopen(badarch, "wb"); assert(af);
    fputs("[ JE : O]\nVERSION = 1\nFORMAT = \"JE\"\nARCH = NOPE\nCODE { EXIT 0 }\n", af);
    fclose(af);
    assert(je_detect_file(badarch) == JE_DETECT_NO);

    const char *exitpath = "je_exit42.jxj";
    FILE *ef = fopen(exitpath, "wb"); assert(ef);
    fputs("[ JE : O]\nVERSION = 1\nFORMAT = \"JE\"\nARCH = X86_64\n\nCODE {\n EXIT 42\n}\n", ef);
    fclose(ef);
    int exit_code = 0;
    assert(je_load_and_run_code(exitpath, &exit_code) == JE_OK);
    assert(exit_code == 42);

    remove(nocode); remove(badjxj); remove(badarch); remove(exitpath);
    remove(in); remove(jx); remove(out); remove(runpath); remove(bare);
    return 0;
}
