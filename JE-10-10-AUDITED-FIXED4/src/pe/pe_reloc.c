#include "je/je_pe.h"
#include <string.h>

static uint16_t rd16(const uint8_t *p){ return (uint16_t)p[0] | ((uint16_t)p[1]<<8); }
static uint32_t rd32(const uint8_t *p){ return (uint32_t)p[0] | ((uint32_t)p[1]<<8) | ((uint32_t)p[2]<<16) | ((uint32_t)p[3]<<24); }

JEStatus je_pe_parse_relocations(const JEPEImage *image, JEPERelocation *out, size_t capacity, size_t *count){
    uint32_t off; size_t n=0;
    if(!image || !out || !count) return JE_ERR_INVALID_ARGUMENT;
    *count=0;
    if(!image->reloc_rva) return JE_OK;
    if(!je_pe_rva_to_offset(image,image->reloc_rva,&off)) return JE_ERR_FORMAT;
    size_t cursor=0;
    while(cursor < image->reloc_size){
        uint64_t bo=(uint64_t)off+cursor;
        if(bo+8>image->size || cursor+8>image->reloc_size) return JE_ERR_FORMAT;
        uint32_t page=rd32(image->data+bo), block_size=rd32(image->data+bo+4);
        if(block_size<8 || cursor+block_size>image->reloc_size) return JE_ERR_FORMAT;
        uint32_t entries=(block_size-8)/2u;
        for(uint32_t i=0;i<entries;i++){
            uint16_t v=rd16(image->data+bo+8+i*2u); uint16_t type=(uint16_t)(v>>12); uint16_t delta=(uint16_t)(v&0x0FFFu);
            if(type==0) continue;
            if(n>=capacity) return JE_ERR_MEMORY;
            out[n].rva=page+delta; out[n].type=type; ++n;
        }
        cursor += block_size;
    }
    *count=n; return JE_OK;
}
