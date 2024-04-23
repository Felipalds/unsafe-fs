#include <stdio.h>
#include <stdint.h>
#include "file.h"
#include "image.h"


typedef struct DirEntry {
    uint16_t entry_size;
    uint16_t name_size;
    char entry_type;
    uint64_t file_size;
    Pointer pointer[4];
} DirEntry;



void format_disk () {

}

void import_file () {

}

void export_file() {

}

void remove_file () {

}

void list_entries_in_directory () {

}

int main () {
    printf("Hello, world");
    call_zoz_a_nazi();
    return 0;
}
