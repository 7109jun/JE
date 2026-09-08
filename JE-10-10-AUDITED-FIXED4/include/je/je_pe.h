#ifndef JE_PE_H
#define JE_PE_H
#include "je.h"
#include <stdint.h>
#include <stddef.h>
#ifdef __cplusplus
extern "C" {
#endif

#define JE_PE_MAX_SECTIONS 96u
#define JE_PE_MAX_IMPORTS 256u
#define JE_PE_MAX_IMPORT_FUNCTIONS 4096u
#define JE_PE_MAX_EXPORTS 4096u
#define JE_PE_MAX_RELOCATIONS 65536u
#define JE_PE_IMAGE_NT_SIGNATURE 0x00004550u
#define JE_PE_MACHINE_I386 0x014Cu
#define JE_PE_MACHINE_AMD64 0x8664u
#define JE_PE_MACHINE_ARMNT 0x01C4u
#define JE_PE_MACHINE_ARM64 0xAA64u

typedef struct JEPESection {
    char name[9];
    uint32_t virtual_size;
    uint32_t virtual_address;
    uint32_t raw_size;
    uint32_t raw_offset;
    uint32_t characteristics;
} JEPESection;

typedef struct JEPEImportFunction {
    uint64_t ordinal;
    char name[256];
    uint32_t thunk_rva;
} JEPEImportFunction;

typedef struct JEPEImport {
    char dll[260];
    uint32_t oft_rva;
    uint32_t ft_rva;
    uint16_t major_version;
    uint16_t minor_version;
    uint32_t first_function;
    uint32_t function_count;
} JEPEImport;

typedef struct JEPEExport {
    uint32_t ordinal;
    uint32_t rva;
    char name[256];
} JEPEExport;

typedef struct JEPERelocation {
    uint32_t rva;
    uint16_t type;
} JEPERelocation;

typedef struct JEPEImage {
    const uint8_t *data;
    size_t size;
    uint64_t image_base;
    uint64_t entry_rva;
    uint32_t size_of_image;
    uint32_t size_of_headers;
    uint32_t file_alignment;
    uint32_t section_alignment;
    uint32_t export_rva, export_size;
    uint32_t import_rva, import_size;
    uint32_t reloc_rva, reloc_size;
    uint16_t machine;
    uint16_t optional_magic;
    uint16_t characteristics;
    uint16_t subsystem;
    uint16_t section_count;
    JEPESection sections[JE_PE_MAX_SECTIONS];
} JEPEImage;

JEStatus je_pe_parse(const void *data, size_t size, JEPEImage *out);
const JEPESection *je_pe_find_section(const JEPEImage *image, const char *name);
int je_pe_rva_to_offset(const JEPEImage *image, uint32_t rva, uint32_t *offset);
JEStatus je_pe_validate(const JEPEImage *image);
JEStatus je_pe_parse_imports(const JEPEImage *image, JEPEImport *out, size_t capacity, size_t *count);
JEStatus je_pe_parse_exports(const JEPEImage *image, JEPEExport *out, size_t capacity, size_t *count);
JEStatus je_pe_parse_relocations(const JEPEImage *image, JEPERelocation *out, size_t capacity, size_t *count);
JEStatus je_pe_write(const JEPEImage *image, const char *path);

#ifdef __cplusplus
}
#endif
#endif
