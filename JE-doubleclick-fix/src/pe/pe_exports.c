#include "je/je_pe.h"
#include <string.h>

static uint16_t rd16(const uint8_t *p){ return (uint16_t)p[0] | ((uint16_t)p[1]<<8); }
static uint32_t rd32(const uint8_t *p){ return (uint32_t)p[0] | ((uint32_t)p[1]<<8) | ((uint32_t)p[2]<<16) | ((uint32_t)p[3]<<24); }
static int read_cstr(const JEPEImage *img,uint32_t rva,char *dst,size_t cap){
    uint32_t off; size_t i=0;
    if(!je_pe_rva_to_offset(img,rva,&off) || cap==0) return 0;
    while((uint64_t)off+i<img->size){
        unsigned char c=img->data[off+i];
        if(i+1>=cap) return 0;
        dst[i]=(char)c;
        if(c==0) return 1;
        ++i;
    }
    return 0;
}

JEStatus je_pe_parse_exports(const JEPEImage *image, JEPEExport *out, size_t capacity, size_t *count){
    uint32_t dir_off, functions_off, names_off, ords_off;
    uint32_t base, nfunc, nname;
    if(!image || !out || !count) return JE_ERR_INVALID_ARGUMENT;
    *count=0;
    if(!image->export_rva) return JE_OK;
    if(!je_pe_rva_to_offset(image,image->export_rva,&dir_off) || (uint64_t)dir_off+40>image->size) return JE_ERR_FORMAT;
    const uint8_t *d=image->data+dir_off;
    base=rd32(d+16); nfunc=rd32(d+20); nname=rd32(d+24);
    if(nfunc>JE_PE_MAX_EXPORTS) nfunc=JE_PE_MAX_EXPORTS;
    if(nname>JE_PE_MAX_EXPORTS) nname=JE_PE_MAX_EXPORTS;
    if(nfunc){ if(!je_pe_rva_to_offset(image,rd32(d+28),&functions_off)) return JE_ERR_FORMAT; }
    else functions_off=0;
    if(nname){ if(!je_pe_rva_to_offset(image,rd32(d+32),&names_off)) return JE_ERR_FORMAT; if(!je_pe_rva_to_offset(image,rd32(d+36),&ords_off)) return JE_ERR_FORMAT; }
    else names_off=ords_off=0;
    if(nfunc>capacity) return JE_ERR_MEMORY;
    for(uint32_t i=0;i<nfunc;i++){
        uint64_t fo=(uint64_t)functions_off+i*4u;
        if(fo+4>image->size) return JE_ERR_FORMAT;
        memset(&out[i],0,sizeof(out[i]));
        out[i].ordinal=base+i;
        out[i].rva=rd32(image->data+fo);
    }
    for(uint32_t i=0;i<nname;i++){
        uint64_t no=(uint64_t)names_off+i*4u, oo=(uint64_t)ords_off+i*2u;
        if(no+4>image->size || oo+2>image->size) return JE_ERR_FORMAT;
        uint32_t nrva=rd32(image->data+no);
        uint16_t idx=rd16(image->data+oo);
        if(idx<nfunc && !read_cstr(image,nrva,out[idx].name,sizeof(out[idx].name))) return JE_ERR_FORMAT;
    }
    *count=nfunc;
    return JE_OK;
}
