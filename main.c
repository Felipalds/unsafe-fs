#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include "file.h"
#include "image.h"
#include <string.h>
#include "consts.h"
#include "utils.h"


typedef struct DirEntry {
    uint16_t entry_size;
    uint16_t name_size;
    char entry_type;
    uint64_t file_size;
    Pointer pointer[4];
} DirEntry;


const char options[4][255] = {
        "OPEN FILE\n",
        "LIST ROOT DIR\n",
        "CREATE IMAGE\n",
        "LIST FREE BLOCKS\n"
};

int main (int argc, char *argv[]) {

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
    file_pointer = fopen(FILENAME, "rb");
    if(file_pointer == NULL) {
       file_pointer = fopen(FILENAME, "w");
       if(file_pointer == NULL)
           print(RED, "Error opening or creating the file");

       char disk_name[255] = "Nome do disco";
       image = image_create(file_pointer, disk_name, 1024, 1024*100);
    } else {
        image = image_open(file_pointer);
    }

    while(1) {
        int option;
        print(BLUE, "============ UNSAFE FS =============\n");
        print(YELLOW, "Made in C - By Rodrigues and Rosa\n");

        for (int c = 0; c < 4; c++) {
            char position[2];
            sprintf(position, "%d", c + 1);
            print(GREEN, position);
            print(WHITE, options[c]);
        }
        scanf("%d", &option);
        switch (option) {
            case 4:
                list_free_blocks(image);
        }
    }

}
