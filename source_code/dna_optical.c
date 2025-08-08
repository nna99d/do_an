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
    FILE *fin = fopen(input, "rb");
    if (!fin) { printf("Khong mo duoc file input\n"); return; }

    char header[3];
    int width, height, maxval;
    fscanf(fin, "%2s\n%d %d\n%d\n", header, &width, &height, &maxval);
    if (strcmp(header, "P5") != 0) { printf("Chi ho tro PGM P5\n"); fclose(fin); return; }

    int size = width * height;
    unsigned char *data = (unsigned char*)malloc(size);
    fread(data, 1, size, fin);
    fclose(fin);

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

    FILE *fout = fopen(output, "wb");
    fprintf(fout, "P5\n%d %d\n255\n", width, height);
    fwrite(enc_data, 1, size, fout);
    fclose(fout);

    FILE *fkey = fopen(keyfile, "w");
    fprintf(fkey, "%d %d\n", width, height);
    for (int i = 0; i < size*4; i++) fputc(optical[i], fkey);
    fclose(fkey);

    printf("Ma hoa xong -> %s, key -> %s\n", output, keyfile);

    free(data); free(optical); free(enc_data);
}

void decrypt_image(const char *input, const char *output, const char *keyfile) {
    FILE *fkey = fopen(keyfile, "r");
    if (!fkey) { printf("Khong mo duoc file key\n"); return; }
    int width, height;
    fscanf(fkey, "%d %d\n", &width, &height);
    int size = width * height;
    char *optical = (char*)malloc(size * 4);
    fread(optical, 1, size*4, fkey);
    fclose(fkey);

    FILE *fin = fopen(input, "rb");
    if (!fin) { printf("Khong mo duoc file input\n"); return; }
    char header[3];
    int w2, h2, maxval;
    fscanf(fin, "%2s\n%d %d\n%d\n", header, &w2, &h2, &maxval);
    if (strcmp(header, "P5") != 0 || w2 != width || h2 != height) {
        printf("File anh khong khop voi key\n");
        fclose(fin); free(optical); return;
    }

    unsigned char *data = (unsigned char*)malloc(size);
    fread(data, 1, size, fin);
    fclose(fin);

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

    FILE *fout = fopen(output, "wb");
    fprintf(fout, "P5\n%d %d\n255\n", width, height);
    fwrite(dec_data, 1, size, fout);
    fclose(fout);

    printf("Giai ma xong -> %s\n", output);

    free(optical); free(data); free(dec_data);
}

int main() {
    int choice;
    char in_file[256], out_file[256], key_file[256];
    printf("Chon che do (1=Ma hoa, 2=Giai ma): ");
    scanf("%d", &choice);

    if (choice == 1) {
        printf("Nhap file anh PGM goc: "); scanf("%s", in_file);
        printf("Nhap file anh ma hoa output: "); scanf("%s", out_file);
        printf("Nhap file key output: "); scanf("%s", key_file);
        encrypt_image(in_file, out_file, key_file);
    } else if (choice == 2) {
        printf("Nhap file anh ma hoa: "); scanf("%s", in_file);
        printf("Nhap file anh giai ma output: "); scanf("%s", out_file);
        printf("Nhap file key: "); scanf("%s", key_file);
        decrypt_image(in_file, out_file, key_file);
    } else {
        printf("Lua chon khong hop le\n");
    }
    return 0;
}
