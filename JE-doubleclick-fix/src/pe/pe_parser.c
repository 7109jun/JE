#include "je/je_pe.h"
#include "je/je_error.h"
#include <string.h>

static int range_ok(size_t size, uint64_t off, uint64_t len) {
    return off <= size && len <= (uint64_t)size - off;
}
static uint16_t rd16(const uint8_t *p){ return (uint16_t)p[0] | ((uint16_t)p[1]<<8); }
static uint32_t rd32(const uint8_t *p){ return (uint32_t)p[0] | ((uint32_t)p[1]<<8) | ((uint32_t)p[2]<<16) | ((uint32_t)p[3]<<24); }
static uint64_t rd64(const uint8_t *p){ return (uint64_t)rd32(p) | ((uint64_t)rd32(p+4)<<32); }

JEStatus je_pe_parse(const void *data, size_t size, JEPEImage *out) {
    const uint8_t *p=(const uint8_t*)data;
    uint64_t nt, opt, sec_table;
    uint16_t nsec, opt_size, magic;
    if(!p || !out) return JE_ERR_INVALID_ARGUMENT;
    memset(out,0,sizeof(*out));
    if(!range_ok(size,0,0x40) || rd16(p)!=0x5A4D) { je_set_error("invalid DOS header"); return JE_ERR_FORMAT; }
    uint32_t lfanew=rd32(p+0x3c);
    nt=lfanew;
    if(!range_ok(size,nt,24) || rd32(p+nt)!=JE_PE_IMAGE_NT_SIGNATURE) { je_set_error("invalid PE signature"); return JE_ERR_FORMAT; }
    uint16_t machine=rd16(p+nt+4);
    nsec=rd16(p+nt+6);
    opt_size=rd16(p+nt+20);
    if(nsec==0 || nsec>JE_PE_MAX_SECTIONS) { je_set_error("invalid section count"); return JE_ERR_FORMAT; }
    if(!range_ok(size,nt+24,opt_size)) { je_set_error("truncated optional header"); return JE_ERR_FORMAT; }
    opt=nt+24; magic=rd16(p+opt);
    if(magic!=0x10B && magic!=0x20B) { je_set_error("unsupported PE optional header"); return JE_ERR_UNSUPPORTED; }
    if(magic==0x20B && opt_size<112) { je_set_error("truncated PE32+ optional header"); return JE_ERR_FORMAT; }
    if(magic==0x10B && opt_size<96) { je_set_error("truncated PE32 optional header"); return JE_ERR_FORMAT; }
    out->data=p; out->size=size; out->machine=machine; out->optional_magic=magic;
    out->section_count=nsec; out->entry_rva=rd32(p+opt+16); out->section_alignment=rd32(p+opt+32); out->file_alignment=rd32(p+opt+36);
    out->size_of_image=rd32(p+opt+56); out->size_of_headers=rd32(p+opt+60); out->subsystem=rd16(p+opt+68);
    out->characteristics=rd16(p+nt+22);
    out->image_base = magic==0x20B ? rd64(p+opt+24) : rd32(p+opt+28);
    uint32_t dir_off=magic==0x20B?112u:96u;
    uint32_t dirs=rd32(p+opt+(magic==0x20B?108:92));
    if(dirs>16) { je_set_error("too many PE data directories"); return JE_ERR_FORMAT; }
    if(dirs && (uint64_t)dir_off + (uint64_t)dirs * 8u > opt_size) {
        je_set_error("PE data directories exceed optional header");
        return JE_ERR_FORMAT;
    }
    if(dirs>0){ out->export_rva=rd32(p+opt+dir_off); out->export_size=rd32(p+opt+dir_off+4); }
    if(dirs>1){ out->import_rva=rd32(p+opt+dir_off+8); out->import_size=rd32(p+opt+dir_off+12); }
    if(dirs>5){ out->reloc_rva=rd32(p+opt+dir_off+40); out->reloc_size=rd32(p+opt+dir_off+44); }

    sec_table=nt+24+opt_size;
    if(!range_ok(size,sec_table,(uint64_t)nsec*40u)) { je_set_error("truncated section table"); return JE_ERR_FORMAT; }
    for(uint16_t i=0;i<nsec;i++){
        const uint8_t *s=p+sec_table+(size_t)i*40u;
        memcpy(out->sections[i].name,s,8); out->sections[i].name[8]='\0';
        out->sections[i].virtual_size=rd32(s+8); out->sections[i].virtual_address=rd32(s+12);
        out->sections[i].raw_size=rd32(s+16); out->sections[i].raw_offset=rd32(s+20); out->sections[i].characteristics=rd32(s+36);
        if(out->sections[i].raw_size && !range_ok(size,out->sections[i].raw_offset,out->sections[i].raw_size)) { je_set_error("section raw data out of bounds"); return JE_ERR_FORMAT; }
    }
    return je_pe_validate(out);
}

JEStatus je_pe_validate(const JEPEImage *image){
    if(!image || !image->data) return JE_ERR_INVALID_ARGUMENT;
    if(image->section_count==0 || image->section_count>JE_PE_MAX_SECTIONS) return JE_ERR_FORMAT;
    if(image->section_alignment==0 || image->file_alignment==0) return JE_ERR_FORMAT;
    if(image->size_of_headers && image->size_of_headers>image->size) return JE_ERR_FORMAT;
    return JE_OK;
}

const JEPESection *je_pe_find_section(const JEPEImage *image, const char *name){
    if(!image||!name) return NULL;
    for(uint16_t i=0;i<image->section_count;i++) if(strncmp(image->sections[i].name,name,8)==0) return &image->sections[i];
    return NULL;
}

int je_pe_rva_to_offset(const JEPEImage *image, uint32_t rva, uint32_t *offset){
    if(!image||!offset) return 0;
    if(rva < image->size_of_headers){ *offset=rva; return rva < image->size; }
    for(uint16_t i=0;i<image->section_count;i++){
        const JEPESection *s=&image->sections[i];
        uint32_t span=s->virtual_size>s->raw_size?s->virtual_size:s->raw_size;
        uint64_t end=(uint64_t)s->virtual_address+span;
        if(rva>=s->virtual_address && (uint64_t)rva<end){
            uint64_t delta=(uint64_t)rva-s->virtual_address;
            if(delta >= s->raw_size) return 0;
            uint64_t off=(uint64_t)s->raw_offset+delta;
            if(off<image->size){*offset=(uint32_t)off;return 1;} return 0;
        }
    }
    return 0;
}

