#include <stdlib.h>
#include <stdio.h>
#include "image.h"


int test_image_create_open() {
    int err = 0;

    FILE *file = fopen("./images/image_create.img", "wb+");
    const char *disk_name = "new_image";
    uint16_t block_size = 1024;
    uint32_t disk_size = 16;
    image_create(file, disk_name, block_size, disk_size);
    fclose(file);

    file = fopen("./images/image_create.img", "r");
    Image image = image_open(file);
    if (strncmp(disk_name, image.meta.disk_name, sizeof(image.meta.disk_name))) {
        fprintf(stderr, "ERRO (image_create/image_open): disk_name não escrito corretamente\n");
        err++;
    }
    if (block_size != image.meta.block_size) {
        fprintf(stderr, "ERRO (image_create/image_open): block_size não escrito corretamente\n");
        err++;
    }
    if (disk_size != image.meta.disk_size) {
        fprintf(stderr, "ERRO (image_create/image_open): disk_size não escrito corretamente\n");
        err++;
    }
    fclose(file);
    return err;
}

int main() {
    int err = 0;
    err += test_image_create_open();

    fprintf(stderr, "%i erros\n", err);
}
