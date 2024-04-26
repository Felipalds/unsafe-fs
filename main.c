#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <inttypes.h> // for SCNu16 and SCNu32
#include "image.h"
#include "file.h"
#include "utils.h"

#define FILE_NOT_PROVIDED_MESSAGE "Filename not provided. Please, run ./main /PATH/TO/FILE.img and try again"

const char *options[7] = {
    "FORMAT DISK",
    "VIEW META",
    "LIST ROOT DIR",
    "OPEN FILE",
    "LIST FREE BLOCKS",
    "IMPORT A FILE FROM SYSTEM",
    "EXPORT A FILE TO SYSTEM"
};

int main(int argc, char *argv[]) {
    system("clear");

    if (argc == 1) {
        cprintf(RED, "%s\n", FILE_NOT_PROVIDED_MESSAGE);
        return 1;
    }
    const char *FILENAME = argv[1];
    if (strlen(FILENAME) == 0 || FILENAME == NULL) {
        cprintf(RED, "%s\n", FILE_NOT_PROVIDED_MESSAGE);
        return 1;
    }

    Image image = { .file = NULL };
    FILE* file_pointer;
    file_pointer = fopen(FILENAME, "rb");
    if (file_pointer == NULL) {
       file_pointer = fopen(FILENAME, "wb");
       if(file_pointer == NULL) {
           cprintf(RED, "Error opening or creating the file");
       }
       const char *disk_name = "Nome do disco";
       image = image_create(file_pointer, disk_name, 1024, 100);
       cprintf(WHITE, "New image created...\n");
    } else {
        image = image_open(file_pointer);
        cprintf(WHITE, "Image opened\n");
    }

    while(1) {
        cprintf(BLUE, "============ UNSAFE FS =============\n");
        cprintf(YELLOW, "Made in C - By Rodrigues and Rosa\n");

        for (int c = 0; c < sizeof(options)/sizeof(*options); c++) {
            cprintf(GREEN, "%2d ", c + 1);
            cprintf(WHITE, "%s\n", options[c]);
        }
        fflush(stdin);
        int option;
        scanf("%d", &option);
        switch (option) {
            case -1:
                if (image.file) {
                    fclose(image.file);
                }
                return 0;
            case 1: {
                char new_disk_name[16];
                uint16_t block_size;
                uint32_t disk_size;
                printf("WHAT WILL BE THE NAME OF YOUR DISK?\n");
                scanf("%s", new_disk_name);
                printf("WHAT WILL BE THE BLOCK SIZE IN BYTES?\n");
                scanf("%"SCNu16"", &block_size);
                printf("WHAT WILL BE THE DISK SIZE IN BYTES?\n");
                scanf("%d"SCNu32"", &disk_size);
                image_create(file_pointer, new_disk_name, block_size, disk_size);
            }
            case 2: {
                view_meta(image.meta);
                break;
            }
            case 3: {
                list_root_dir();
                break;
            }
            case 5: {
                list_free_blocks(image);
                break;
            }
            default: {
                system("clear");
                cprintf(RED, "Type a valid option!\n");
                break;
            }
        }
        cprintf(BLUE, "Press any key to continue...\n");
        fflush(stdin);
        scanf("%*c");
    }
}

