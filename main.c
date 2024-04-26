#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "image.h"
#include "file.h"
#include "utils.h"

#define FILE_NOT_PROVIDED_MESSAGE "Filename not provided. Please, run ./main /PATH/TO/FILE.img and try again"

const char *options[4] = {
    "OPEN FILE",
    "LIST ROOT DIR",
    "CREATE IMAGE",
    "LIST FREE BLOCKS"
};

int main (int argc, char *argv[]) {
    if (argc <= 1) {
        cprintf(RED, "%s\n", FILE_NOT_PROVIDED_MESSAGE);
        return 1;
    }

    const char *filename = argv[1];
    if (strlen(filename) == 0 || filename == NULL) {
        cprintf(RED, "%s\n", FILE_NOT_PROVIDED_MESSAGE);
        return 1;
    }

    // TODO: mover toda essa lógica para o loop interativo
    Image image;
    FILE* file_pointer;
    file_pointer = fopen(filename, "rb");
    if (file_pointer == NULL) {
       file_pointer = fopen(filename, "wb");
       if (file_pointer == NULL) {
           cprintf(RED, "Error opening or creating '%s'\n", filename);
       }
       const char *disk_name = "Nome do disco";
       image = image_create(file_pointer, disk_name, 1024, 100);
    } else {
        image = image_open(file_pointer);
    }

    while (1) {
        int option;
        cprintf(BLUE, "============ UNSAFE FS =============\n");
        cprintf(YELLOW, "Made in C - By Rodrigues and Rosa\n");

        for (int c = 0; c < sizeof(options)/sizeof(*options); c++) {
            cprintf(GREEN, "%2d ", c + 1);
            cprintf(WHITE, "%s\n", options[c]);
        }
        if (scanf("%d", &option) != 1) {
            break;
        };
        switch (option) {
            case 4:
                list_free_blocks(image);
                break;
        }
    }
    fclose(image.file);
    return 0;
}
