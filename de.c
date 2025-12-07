#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

static inline uint8_t rotr8(uint8_t v,unsigned r){ r&=7; return (uint8_t)((v>>r)|(v<<(8-r))); }

// DNA
static inline char bits_to_base(uint8_t b2){ switch(b2&3){case 0: return 'A'; case 1: return 'C'; case 2: return 'G'; default: return 'T';}}
static inline uint8_t base_to_bits(char base){ switch(base){case 'A':return 0;case 'C':return 1;case 'G':return 2;case 'T':return 3; default:return 0;}}

void bytes_to_bases(const uint8_t *in, size_t len, char *out){
    for(size_t i=0;i<len;i++){
        uint8_t v=in[i];
        out[i*4+0]=bits_to_base((v>>6)&3);
        out[i*4+1]=bits_to_base((v>>4)&3);
        out[i*4+2]=bits_to_base((v>>2)&3);
        out[i*4+3]=bits_to_base(v&3);
    }
}

void bases_to_bytes(const char *in, size_t base_count, uint8_t *out){
    size_t bytes=base_count/4;
    for(size_t i=0;i<bytes;i++){
        uint8_t b0=base_to_bits(in[i*4+0]);
        uint8_t b1=base_to_bits(in[i*4+1]);
        uint8_t b2=base_to_bits(in[i*4+2]);
        uint8_t b3=base_to_bits(in[i*4+3]);
        out[i]=(b0<<6)|(b1<<4)|(b2<<2)|b3;
    }
}

void dna_complement(char *bases, size_t n){
    for(size_t i=0;i<n;i++){
        switch(bases[i]){
            case 'A': bases[i]='T'; break;
            case 'T': bases[i]='A'; break;
            case 'C': bases[i]='G'; break;
            case 'G': bases[i]='C'; break;
        }
    }
}

void dna_reverse(char *bases, size_t n){
    for(size_t i=0;i<n/2;i++){
        char t=bases[i]; bases[i]=bases[n-1-i]; bases[n-1-i]=t;
    }
}

void dna_swap_pairs(char *bases, size_t n){
    for(size_t i=0;i+1<n;i+=2){
        char t=bases[i]; bases[i]=bases[i+1]; bases[i+1]=t;
    }
}

//  Header 
#pragma pack(push,1)
typedef struct {
    uint32_t magic; 
    uint32_t width;
    uint32_t height;
    uint8_t channels;
    uint8_t reserved[3];
} enc_header_t;
#pragma pack(pop)

// Main 
int main(void){
    char encfile[256], keyfile[256], outimg[256];
    printf("Nhap file ma hoa (.enc): "); scanf("%255s", encfile);
    printf("Nhap file key (.bin): "); scanf("%255s", keyfile);
    printf("Nhap ten file anh dau ra (.png): "); scanf("%255s", outimg);

    FILE *fenc=fopen(encfile,"rb");
    FILE *fkey=fopen(keyfile,"rb");
    if(!fenc||!fkey){ fprintf(stderr,"Khong the mo file\n"); return 1;}

    enc_header_t hdr_enc;
    if(fread(&hdr_enc,sizeof(hdr_enc),1,fenc)!=1){ fprintf(stderr,"Loi\n"); return 2;}
    fseek(fkey,sizeof(hdr_enc),SEEK_SET); 

    size_t total_bytes=(size_t)hdr_enc.width*hdr_enc.height*hdr_enc.channels;
    uint8_t *cipherbuf=malloc(1<<16);
    uint8_t *keybuf=malloc(1<<16);
    char *basesbuf=malloc((1<<16)*4);
    uint8_t *outbuf=malloc(total_bytes);
    if(!cipherbuf||!keybuf||!basesbuf||!outbuf){ fprintf(stderr,"OOM\n"); return 3;}

    size_t processed=0;
    const size_t CHUNK=1<<16;

    while(processed<total_bytes){
        size_t toproc=total_bytes-processed; if(toproc>CHUNK) toproc=CHUNK;
        if(fread(cipherbuf,1,toproc,fenc)!=toproc || fread(keybuf,1,toproc,fkey)!=toproc){ fprintf(stderr,"Loi doc\n"); return 4;}

        // reverse rotate + XOR 
        for(size_t i=0;i<toproc;i++){
            uint8_t c=cipherbuf[i], r=keybuf[i];
            cipherbuf[i]=rotr8(c^r,r&0x07);
        }

        // DNA decode
        bytes_to_bases(cipherbuf,toproc,basesbuf);
        for(size_t i=0;i<toproc;i++){
            uint8_t k=keybuf[i];
            size_t bstart=i*4;
            switch(k&3){
                case 0: dna_complement(&basesbuf[bstart],4); break;
                case 1: dna_reverse(&basesbuf[bstart],4); break;
                case 2: dna_swap_pairs(&basesbuf[bstart],4); break;
                case 3: break;
            }
        }
        bases_to_bytes(basesbuf,toproc*4,&outbuf[processed]);
        processed+=toproc;
    }

    stbi_write_png(outimg,hdr_enc.width,hdr_enc.height,hdr_enc.channels,outbuf,hdr_enc.width*hdr_enc.channels);
    printf("Decrypted image -> %s\n", outimg);

    free(cipherbuf); free(keybuf); free(basesbuf); free(outbuf);
    fclose(fenc); fclose(fkey);
    return 0;
}
