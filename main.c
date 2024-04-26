#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include "file.h"
#include "image.h"
#include <string.h>
#include "consts.h"
#include "utils.h"
#include <inttypes.h> // for SCNu16 and SCNu32



typedef struct DirEntry {
    uint16_t entry_size;
    uint16_t name_size;
    char entry_type;
    uint64_t file_size;
    Pointer pointer[4];
} DirEntry;


const char options[7][255] = {
        "FORMAT DISK\n",
        "VIEW META\n",
        "LIST ROOT DIR\n",
        "OPEN FILE\n",
        "LIST FREE BLOCKS\n",
        "IMPORT A FILE FROM SYSTEM\n",
        "EXPORT A FILE TO SYSTEM\n"

};

int main (int argc, char *argv[]) {

    system("clear");
    Image image;

    if (argc == 1) {
        print(RED, FILE_NOT_PROVIDED_MESSAGE);
        exit(1);
    }

    const char *FILENAME = argv[1];
    if (strlen(FILENAME) == 0 || FILENAME == NULL) {
        print(RED, FILE_NOT_PROVIDED_MESSAGE);
        exit(1);
    }

    FILE* file_pointer;
    file_pointer = fopen(FILENAME, "rb+");
    if(file_pointer == NULL) {
       file_pointer = fopen(FILENAME, "w");
       if(file_pointer == NULL) {
           print(RED, "Error opening or creating the file");
       }
       char disk_name[255] = "Nome do disco";
       image = image_create(file_pointer, disk_name, 1024, 1024*100);
       print(WHITE, "New image created...\n");
    } else {
        image = image_open(file_pointer);
        print(WHITE, "Image opened\n");
    }
    char a;

    while(1) {
        int option;
        print(BLUE, "============ UNSAFE FS =============\n");
        print(YELLOW, "Made in C - By Rodrigues and Rosa\n");

        for (int c = 0; c < 7; c++) {
            char position[2];
            sprintf(position, "%d", c + 1);
            print(GREEN, position);
            print(WHITE, options[c]);
        }
        fflush(stdin);
        scanf("%d", &option);
        switch (option) {
            case -1:
                exit(0);
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
                view_meta(image);
                break;
            }
            case 3: {
                list_root_dir();
                break;
            }
            case 5:
                list_free_blocks(image);
                break;
            default:
                system("clear");
                print(RED, "Type a valid option!\n");
        }
        print(BLUE, "Press any key to continue...\n");
        fflush(stdin);
        scanf("%c", &a);
    }

}
