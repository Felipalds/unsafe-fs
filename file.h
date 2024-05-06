#include <stdint.h>
#include <inttypes.h>

typedef struct __attribute__((__packed__)) {
    uint64_t file_size;
    Pointer pointer;
    char type;
    char name[256];
} DirEntry;

int delete_entry (Image image, char entry_name[256]) {

}

void export_file(Image image, DirEntry entry, FILE *out_file) {
    if (entry.file_size == 0) {
        return;
    }
    uint64_t rem_bytes = entry.file_size;
    BlockIter it = block_iter(image, entry.pointer);
    void *buf = next_block(&it, NULL);
    for (char *data = buf; data != NULL; data = next_block(&it, data)) {
        if (rem_bytes >= image.meta.block_size) {
            fwrite(data, image.meta.block_size, 1, out_file);
            rem_bytes -= image.meta.block_size;
        } else {
            fwrite(data, rem_bytes, 1, out_file);
        }
    }
    free(it.pointer_block);
    free(buf);
}

void print_entry(DirEntry entry) {
    printf("%s --- ", entry.name);
    printf("%"SCNu64" bytes\n", entry.file_size);
}

void list_root_dir(Image image) {
    DirEntry dir_entry;
    fseek(image.file, image.meta.block_size, SEEK_SET);
    while(fread(&dir_entry, sizeof(DirEntry), 1, image.file) && dir_entry.file_size > 0 ) {
        if(dir_entry.type != 'D'){
            print_entry(dir_entry);
        }
    }
}

Pointer get_last_root_dir_pos ( Image image ) {
    fseek(image.file, image.meta.block_size, SEEK_SET);

    DirEntry entry;

    while(fread(&entry, sizeof(DirEntry), 1, image.file) && entry.file_size > 0) {
        fread(&entry, sizeof(DirEntry), 1, image.file);
    }
    // TODO: maybe here is not this way
    return ftell(image.file) - sizeof(DirEntry);
}


int import_file (Image image, FILE* new_file, char file_name[256]) {
    fseek(new_file, 0L, SEEK_END);
    uint64_t file_size = ftell(new_file);
    printf("File size is %"SCNu64" bytes\n", file_size);
    Pointer main_pointer = alloc_block(image);
    fseek(new_file, 0, SEEK_SET);

    if(!main_pointer) {
        fprintf(stderr, "Main pointer not allocd successfully!\n");
        return 1;
    }
    printf("Allocd at %"SCNu32" pointer\n", main_pointer);

    char *data_block = malloc(image.meta.block_size);
    fseek(image.file, main_pointer, SEEK_SET);
    int offset = 1;
    uint64_t read_bytes = 0;

    do {
        read_bytes += fread(data_block, 1, image.meta.block_size, new_file);
        Pointer pointer = alloc_block(image);
        write_block(image, pointer, data_block, image.meta.block_size);
        write_pointer_block(image, main_pointer, pointer, offset);
        offset++;
    } while(read_bytes < file_size);

    DirEntry new_entry = { file_size, main_pointer, 'F'};
    strcpy(new_entry.name, file_name);

    Pointer last_root_dir_pos = get_last_root_dir_pos(image);
    fseek(image.file, last_root_dir_pos, SEEK_SET);
    fwrite(&new_entry, sizeof(DirEntry), 1, image.file);

    printf("File written with %"SCNu64" bytes size at %"SCNu32" position in root dir \n", file_size, last_root_dir_pos);
    return 0;
}

DirEntry find_by_name (Image image, char name[256], int* err) {
    DirEntry entry;
    fseek(image.file, image.meta.block_size, SEEK_SET);
    while(fread(&entry, sizeof(DirEntry), 1, image.file) && strcmp(entry.name, name));
    return entry;
}
