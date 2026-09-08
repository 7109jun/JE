#include "je/je_pe.h"
#include "je/je_error.h"
#include <string.h>

static uint16_t rd16(const uint8_t *p){ return (uint16_t)p[0] | ((uint16_t)p[1]<<8); }
static uint32_t rd32(const uint8_t *p){ return (uint32_t)p[0] | ((uint32_t)p[1]<<8) | ((uint32_t)p[2]<<16) | ((uint32_t)p[3]<<24); }
static uint64_t rd64(const uint8_t *p){ return (uint64_t)rd32(p) | ((uint64_t)rd32(p+4)<<32); }
static int cstr_ok(const JEPEImage *img, uint32_t rva, char *dst, size_t cap){
    uint32_t off; size_t i=0;
    if(!je_pe_rva_to_offset(img,rva,&off)) return 0;
    while((uint64_t)off+i<img->size){
        unsigned char c=img->data[off+i];
        if(i+1>=cap) return 0;
        dst[i]=(char)c;
        if(c==0){ return 1; }
        ++i;
    }
    return 0;
}

JEStatus je_pe_parse_imports(const JEPEImage *image, JEPEImport *out, size_t capacity, size_t *count){
    uint32_t off;
    size_t n=0;
    if(!image || !out || !count) return JE_ERR_INVALID_ARGUMENT;
    *count=0;
    if(!image->import_rva) return JE_OK;
    if(!je_pe_rva_to_offset(image,image->import_rva,&off)) return JE_ERR_FORMAT;
    for(size_t i=0;i<JE_PE_MAX_IMPORTS;i++){
        uint64_t d=(uint64_t)off+i*20u;
        if(d+20>image->size) return JE_ERR_FORMAT;
        const uint8_t *p=image->data+d;
        uint32_t oft=rd32(p), name_rva=rd32(p+12), ft=rd32(p+16);
        if(!oft && !name_rva && !ft) break;
        if(n>=capacity) return JE_ERR_MEMORY;
        JEPEImport *imp=&out[n]; memset(imp,0,sizeof(*imp));
        imp->oft_rva=oft; imp->ft_rva=ft;
        imp->major_version=rd16(p+8); imp->minor_version=rd16(p+10);
        if(!cstr_ok(image,name_rva,imp->dll,sizeof(imp->dll))) return JE_ERR_FORMAT;
        uint32_t thunk_rva=oft?oft:ft;
        uint32_t thunk_off;
        if(!je_pe_rva_to_offset(image,thunk_rva,&thunk_off)) return JE_ERR_FORMAT;
        uint32_t width=(image->optional_magic==0x20B)?8u:4u;
        size_t functions=0;
        for(size_t j=0;j<JE_PE_MAX_IMPORT_FUNCTIONS;j++){
            uint64_t to=(uint64_t)thunk_off+j*width;
            if(to+width>image->size) return JE_ERR_FORMAT;
            uint64_t tv=width==8?rd64(image->data+to):rd32(image->data+to);
            if(tv==0) break;
            functions++;
        }
        imp->first_function=(uint32_t)n;
        imp->function_count=(uint32_t)functions;
        ++n;
    }
    *count=n;
    return JE_OK;
}
