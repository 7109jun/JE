#include "je/je_convert.h"
#include "je/je_file.h"
#include "je/je_pe.h"
#include "je/je_error.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static const char hex[] = "0123456789ABCDEF";

JEStatus je_pe_to_je(const char *pe_path, const char *je_path) {
    JEFileBuffer in = {0};
    JEPEImage pe;
    FILE *f = NULL;
    if (!pe_path || !je_path) return JE_ERR_INVALID_ARGUMENT;
    JEStatus st = je_file_read_all(pe_path, &in);
    if (st != JE_OK) return st;
    st = je_pe_parse(in.data, in.size, &pe);
    if (st != JE_OK) { je_file_buffer_free(&in); return st; }

    f = fopen(je_path, "wb");
    if (!f) { je_file_buffer_free(&in); je_set_error("cannot create JE file"); return JE_ERR_IO; }

    fprintf(f, "%s\n\n", JE_MAGIC);
    fprintf(f, "AUTHOR = \"JE converter\"\n");
    fprintf(f, "FORMAT = \"JE\"\n");
    fprintf(f, "VERSION = 1\n");
    if (pe.machine == JE_PE_MACHINE_AMD64) fprintf(f, "ARCH = X86_64\n\n");
    else if (pe.machine == JE_PE_MACHINE_I386) fprintf(f, "ARCH = X86\n\n");
    else if (pe.machine == JE_PE_MACHINE_ARM64) fprintf(f, "ARCH = ARM64\n\n");
    else fprintf(f, "ARCH = UNKNOWN\n\n");

    fprintf(f, "SYSTEM {\n");
    fprintf(f, "    ENTRY_RVA = 0x%llX\n", (unsigned long long)pe.entry_rva);
    fprintf(f, "    IMAGE_BASE = 0x%llX\n", (unsigned long long)pe.image_base);
    fprintf(f, "    SUBSYSTEM = %u\n", (unsigned)pe.subsystem);
    fprintf(f, "    MACHINE = 0x%04X\n", (unsigned)pe.machine);
    fprintf(f, "    SECTION_COUNT = %u\n", (unsigned)pe.section_count);
    fprintf(f, "}\n\n");

    fprintf(f, "PE_METADATA {\n");
    fprintf(f, "    SIZE_OF_IMAGE = 0x%X\n", pe.size_of_image);
    fprintf(f, "    SIZE_OF_HEADERS = 0x%X\n", pe.size_of_headers);
    fprintf(f, "    SECTION_ALIGNMENT = 0x%X\n", pe.section_alignment);
    fprintf(f, "    FILE_ALIGNMENT = 0x%X\n", pe.file_alignment);
    fprintf(f, "}\n\n");

    fprintf(f, "SECTIONS {\n");
    for (uint16_t i = 0; i < pe.section_count; ++i) {
        const JEPESection *s = &pe.sections[i];
        fprintf(f, "    SECTION %u \"%s\" {\n", (unsigned)i, s->name);
        fprintf(f, "        RVA = 0x%X\n", s->virtual_address);
        fprintf(f, "        VIRTUAL_SIZE = 0x%X\n", s->virtual_size);
        fprintf(f, "        RAW_OFFSET = 0x%X\n", s->raw_offset);
        fprintf(f, "        RAW_SIZE = 0x%X\n", s->raw_size);
        fprintf(f, "        CHARACTERISTICS = 0x%X\n", s->characteristics);
        fprintf(f, "    }\n");
    }
    fprintf(f, "}\n\n");

    fprintf(f, "PE_RAW_HEX {\n");
    for (size_t i = 0; i < in.size; ++i) {
        if ((i % 32u) == 0) fputs("    ", f);
        fputc(hex[in.data[i] >> 4], f);
        fputc(hex[in.data[i] & 0x0F], f);
        if ((i % 32u) == 31u || i + 1 == in.size) fputc('\n', f);
        else fputc(' ', f);
    }
    fputs("}\n", f);
    if (fclose(f) != 0) st = JE_ERR_IO;
    je_file_buffer_free(&in);
    if (st != JE_OK) je_set_error("failed writing JE file");
    return st;
}
