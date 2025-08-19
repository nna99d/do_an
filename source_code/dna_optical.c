#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

// DNA mapping
char dna_map[4] = {'A','C','G','T'};

int bits_to_index(int b1, int b0) {
    return (b1 << 1) | b0;
}

void byte_to_dna(unsigned char byte, char *dna) {
    for (int i = 0; i < 4; i++) {
        int b1 = (byte >> (7 - i*2)) & 1;
        int b0 = (byte >> (6 - i*2)) & 1;
        dna[i] = dna_map[bits_to_index(b1, b0)];
    }
}

unsigned char dna_to_byte(char *dna) {
    unsigned char byte = 0;
    for (int i = 0; i < 4; i++) {
        int idx;
        switch (dna[i]) {
            case 'A': idx = 0; break;
            case 'C': idx = 1; break;
            case 'G': idx = 2; break;
            case 'T': idx = 3; break;
            default: idx = 0;
        }
        byte |= ((idx >> 1) & 1) << (7 - i*2);
        byte |= (idx & 1) << (6 - i*2);
    }
    return byte;
}

char xor_base(char a, char b) {
    int idx_a, idx_b;
    switch (a) { case 'A': idx_a=0; break; case 'C': idx_a=1; break; case 'G': idx_a=2; break; case 'T': idx_a=3; break; default: idx_a=0; }
    switch (b) { case 'A': idx_b=0; break; case 'C': idx_b=1; break; case 'G': idx_b=2; break; case 'T': idx_b=3; break; default: idx_b=0; }
    int idx_res = idx_a ^ idx_b;
    return dna_map[idx_res];
}

void encrypt_image(const char *input, const char *output, const char *keyfile) {
    int width, height, channels;
    unsigned char *data = stbi_load(input, &width, &height, &channels, 0);
    if (!data) { printf("Khong doc duoc anh input\n"); return; }

    int size = width * height * channels;
    srand(time(NULL));
    char *optical = (char*)malloc(size * 4);
    for (int i = 0; i < size*4; i++) {
        optical[i] = dna_map[rand() % 4];
    }

    unsigned char *enc_data = (unsigned char*)malloc(size);
    char dna_pixel[4], dna_key[4], dna_res[4];
    for (int i = 0; i < size; i++) {
        byte_to_dna(data[i], dna_pixel);
        memcpy(dna_key, &optical[i*4], 4);
        for (int j = 0; j < 4; j++) {
            dna_res[j] = xor_base(dna_pixel[j], dna_key[j]);
        }
        enc_data[i] = dna_to_byte(dna_res);
    }

    stbi_write_png(output, width, height, channels, enc_data, width * channels);

    FILE *fkey = fopen(keyfile, "w");
    fprintf(fkey, "%d %d %d\n", width, height, channels);
    for (int i = 0; i < size*4; i++) fputc(optical[i], fkey);
    fclose(fkey);

    printf("Ma hoa xong -> %s, key -> %s\n", output, keyfile);

    stbi_image_free(data);
    free(optical); free(enc_data);
}

void decrypt_image(const char *input, const char *output, const char *keyfile) {
    FILE *fkey = fopen(keyfile, "r");
    if (!fkey) { printf("Khong mo duoc file key\n"); return; }
    int width, height, channels;
    fscanf(fkey, "%d %d %d\n", &width, &height, &channels);
    int size = width * height * channels;
    char *optical = (char*)malloc(size * 4);
    fread(optical, 1, size*4, fkey);
    fclose(fkey);

    int w2,h2,c2;
    unsigned char *data = stbi_load(input, &w2, &h2, &c2, 0);
    if (!data || w2!=width || h2!=height || c2!=channels) {
        printf("Anh va key khong khop!\n");
        if (data) stbi_image_free(data);
        free(optical);
        return;
    }

    unsigned char *dec_data = (unsigned char*)malloc(size);
    char dna_pixel[4], dna_key[4], dna_res[4];
    for (int i = 0; i < size; i++) {
        byte_to_dna(data[i], dna_pixel);
        memcpy(dna_key, &optical[i*4], 4);
        for (int j = 0; j < 4; j++) {
            dna_res[j] = xor_base(dna_pixel[j], dna_key[j]);
        }
        dec_data[i] = dna_to_byte(dna_res);
    }

    stbi_write_png(output, width, height, channels, dec_data, width*channels);

    printf("Giai ma xong -> %s\n", output);

    stbi_image_free(data);
    free(optical); free(dec_data);
}

int main() {
    int choice;
    char in_file[256], out_file[256], key_file[256];
    printf("Chon che do (1=Ma hoa, 2=Giai ma): ");
    scanf("%d", &choice);

    if (choice == 1) {
        printf("Nhap file anh PNG/JPG goc: "); scanf("%s", in_file);
        printf("Nhap file anh ma hoa output (PNG): "); scanf("%s", out_file);
        printf("Nhap file key output: "); scanf("%s", key_file);
        encrypt_image(in_file, out_file, key_file);
    } else if (choice == 2) {
        printf("Nhap file anh da ma hoa: "); scanf("%s", in_file);
        printf("Nhap file anh giai ma output (PNG): "); scanf("%s", out_file);
        printf("Nhap file key: "); scanf("%s", key_file);
        decrypt_image(in_file, out_file, key_file);
    } else {
        printf("Lua chon khong hop le\n");
    }
    return 0;
}
