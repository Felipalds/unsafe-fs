#include <stdlib.h>
#include <stdio.h>
#include "image.h"
#include "consts.h"
#include "utils.h"


int test_image_create_open(int *err, int *oks) {
    FILE *file = fopen("./images/image_create.img", "wb+");
    const char *disk_name = "new_image";
    uint16_t block_size = 1024;
    uint32_t disk_size = 16;
    image_create(file, disk_name, block_size, disk_size);
    fclose(file);

    file = fopen("./images/image_create.img", "r");
    Image image = image_open(file);
    if (strncmp(disk_name, image.meta.disk_name, sizeof(image.meta.disk_name))) {
        fprintf(stderr, "☠ ERRO (image_create/image_open): disk_name não escrito corretamente\n");
        *err++;
    } else {
        *oks++;
        success("✓ Image disk created successfully");
    }

    if (block_size != image.meta.block_size) {
        fprintf(stderr, "☠ ERRO (image_create/image_open): block_size não escrito corretamente\n");
        err++;
    } else {
        (*oks)++;
        success("✓ Image disk created successfully");
    }
    if (disk_size != image.meta.disk_size) {
        fprintf(stderr, "☠ ERRO (image_create/image_open): disk_size não escrito corretamente\n");
        err++;
    }else {
        (*oks)++;
        success("✓ Image disk created successfully");
    }
    
    fseek(image.file, sizeof(image.meta), SEEK_SET);
    Pointer free_pointers[4];
    if (fread(free_pointers, sizeof(free_pointers), 1, image.file) != 1) {
        fprintf(stderr, "☠ ERRO (image_create/image_open): erro de leitura nos free pointers\n");
        err++;
    } else {
        oks++;
        success("✓ Image disk created successfully");
    }
    Pointer reference_pointers[4] = { 2, image.meta.disk_size-1, 0, 0 };
    if (memcmp(reference_pointers, free_pointers, sizeof(free_pointers))) {
        fprintf(stderr, "☠ ERRO (image_create/image_open): free pointers não escritos corretamente\n");
        err++;
    } else{
        oks++;
        success("✓ Image disk created successfully");
    }

    fseek(image.file, image.meta.block_size, SEEK_SET);
    Pointer p;
    if (fread(&p, sizeof(p), 1, image.file) != 1) {
        fprintf(stderr, "☠ ERRO (image_create/image_open): erro de leitura no bloco 1\n");
        err++;
    } else {
        oks++;
        success("✓ Image disk created successfully");
    }
    if (p != 0) {
        fprintf(stderr, "☠ ERRO (image_create/image_open): primeiro ponteiro do bloco 1 não escrito corretamente\n");
        err++;
    }
    else {
        oks++;
        success("✓ Image disk created successfully");
    }

    fclose(file);
}

int test_alloc_free_block(int *err, int *oks) {

    FILE *file = fopen("./images/image_create.img", "wb+");
    const char *disk_name = "new_image";
    uint16_t block_size = 1024;
    uint32_t disk_size = 16;
    Image image = image_create(file, disk_name, block_size, disk_size);
    
    for (Pointer p = 2; p < 16; p++) {
        if (p != alloc_block(image)) {
            fprintf(stderr, "☠ ERRO (alloc_block/free_block): Comportamento inesperado na alocação\n");
            err++;
        }
       else {
           oks++;
            success("✓ Image disk created successfully");
       }
    }
    
    Pointer fail = alloc_block(image);
    if (fail != 0) {
        fprintf(stderr, "☠ ERRO (alloc_block/free_block): Não há blocos livres, mas alloc_block retornou não-nulo (%u)\n", fail);
        err++;
    } else {

        success("✓ Image disk created successfully");
        oks++;
    }

    for (Pointer p = 15; p >= 2; p--) {
        if (free_block(image, p)) {
            fprintf(stderr, "☠ ERRO (alloc_block/free_block): Bloco não foi liberado como esperado\n");
            err++;
        } else {

            success("✓ Image disk created successfully");
            oks++;
        }
    }

    for (Pointer p = 2; p < 16; p++) {
        if (p != alloc_block(image)) {
            fprintf(stderr, "☠ ERRO (alloc_block/free_block): Comportamento inesperado na alocação após liberamento\n");
            err++;
            break;
        } else {
            oks++;
            success("✓ Image disk created successfully");
        }
    }
    
}

int main() {
    int err = 0;
    int oks = 0;
    test_image_create_open(&err, &oks);
    test_alloc_free_block(&err, &oks);

    fprintf(stderr, "%i erros\n", err);
    char success [255];

    sprintf(success, "%d tests passeds\n", oks);
    print(GREEN, success);
}
