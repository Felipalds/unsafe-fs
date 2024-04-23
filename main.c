#include <stdio.h>
#include <stdint.h>

typedef uint32_t Pointer;

typedef struct MetaBlock {
    char disk_name[16];
    uint16_t block_size;
    uint32_t disk_size;
} MetaBlock;

typedef struct Interval {
    Pointer begin_pointer;
    Pointer end_pointer;
} Interval;

typedef struct DirEntry {
    uint16_t entry_size;
    uint16_t name_size;
    char entry_type;
    uint64_t file_size;
    Pointer pointer[4];
} DirEntry;


typedef struct Image {
    MetaBlock meta;
    FILE* file;
} Image;

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

void read_meta() {

}

int main () {
    printf("Hello, world");
    return 0;
}
