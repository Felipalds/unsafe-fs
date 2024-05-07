#include <stdint.h>
#include <inttypes.h>

typedef struct __attribute__((__packed__)) {
    uint64_t file_size;
    Pointer pointer;
    char type;
    char name[256];
} DirEntry;

int delete_entry(Image image, const char *entry_name) {
    /*
     *  Pointer entry_block;
     *  size_t entry_offset;
     *  DirEntry entry = find_entry_by_name(entry_name);
     *  if (entry.type == 'D') {
     *      return 1;
     *  }
     *  // escreve novo entry como tipo 'D'
     *  uint64_t offset = entry_block*image.meta.block_size + entry_offset*sizeof(DirEntry);
     *  entry.type = 'D';
     *  fseek(image.file, offset, SEEK_SET);
     *  fwrite(&entry, sizeof(entry), 1, image.file);
     *  // libera todos os blocos do pointer block
     *  Pointer *pointer_block = read_block(image, entry.pointer, NULL);
     *  Pointer *end = &pointer_block[image.meta.block_size / sizeof(Pointer)];
     *  for (Pointer *pointer = pointer_block; pointer < end; pointer++) {
     *      free_block(image, *pointer);
     *  }
     *  // e libera próprio pointer block
     *  free_block(image, entry.pointer);
     *  free(pointer_block);
     *  return 0;
     */
}

void export_file(Image image, DirEntry entry, FILE *out_file) {
    if (entry.file_size == 0) {
        return;
    }
    uint64_t rem_bytes = entry.file_size;
    Pointer *pointer_block = read_block(image, entry.pointer, NULL);
    char *data_block = NULL;
    size_t num_pointers = image.meta.block_size/sizeof(Pointer);
    for (size_t block = 0; block < num_pointers && pointer_block[block]; block++) {
        data_block = read_block(image, pointer_block[block], data_block);
        if (rem_bytes >= image.meta.block_size) {
            fwrite(data_block, image.meta.block_size, 1, out_file);
            rem_bytes -= image.meta.block_size;
        } else {
            fwrite(data_block, rem_bytes, 1, out_file);
            break;
        }
    }
    free(data_block);
    free(pointer_block);
}

void print_entry(DirEntry entry) {
    printf("%s --- ", entry.name);
    printf("%"SCNu64" bytes\n", entry.file_size);
}

void list_root_dir(Image image) {
    DirEntry *data_block = NULL;
    Pointer *pointer_block = read_block(image, 1, NULL);
    size_t block = 0;
    size_t entry;
    while (pointer_block[block] && block < image.meta.block_size / sizeof(Pointer)) {
        data_block = read_block(image, pointer_block[block], data_block);
        entry = 0;
        while (entry < image.meta.block_size / sizeof(DirEntry)) {
            if (data_block[entry].type != 'D') {
                print_entry(data_block[entry]);
            }
            entry++;
        }
        block++;
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

DirEntry find_by_name (Image image, char name[256], int* err, Pointer* pointer,  int* offset) {
    DirEntry found_entry;
    fseek(image.file, image.meta.block_size, SEEK_SET);
    size_t entry = 0;
    DirEntry *data_block = NULL;
    Pointer *pointer_block = read_block(image, 1, NULL);
    size_t block = 0;
    while (pointer_block[block] && block < image.meta.block_size / sizeof(Pointer)) {
        data_block = read_block(image, pointer_block[block], data_block);
        entry = 0;
        while (entry < image.meta.block_size / sizeof(DirEntry)) {
            if (data_block[entry].type != 'D' && !strcmp(name, found_entry.name)) {
                found_entry = data_block[entry];
                break;
            }
            entry++;
        }
        block++;
    }
    free(data_block);
    free(pointer_block);
    return found_entry;
}

int clean_block(Image image, Pointer p) {
    fseek(image.file, p, SEEK_SET);
    for(int c = 0; c < image.meta.block_size; c++) {
        fputc(0, image.file);
    }
}

void update_entry (Image image, DirEntry entry, int offset) {
    fseek(image.file, image.meta.block_size + offset * sizeof(DirEntry), SEEK_SET);
}

//int delete_entry (Image image, char entry_name[256]) {
//    int find_errors = 0;
//    int offset = 0;
//    DirEntry entry = find_by_name(image, entry_name, &find_errors, &offset);
//    if (find_errors > 0) {
//        fprintf(stderr, "%d errors findind the name of file\n", find_errors);
//    }
//    Pointer main_pointer = entry.pointer;
//
//    fseek(image.file, main_pointer, SEEK_SET);
//    int block_read = 0;
//    Pointer p;
//
//    while (block_read < sizeof(Pointer)) {
//        fread(&p, sizeof (Pointer), 1, image.file);
//        free_block(image, p);
//        block_read+=sizeof (Pointer);
//    }
//
//    fseek(image.file, main_pointer, SEEK_SET);
//    free_block(image, main_pointer);
//    clean_block(image, main_pointer);
//
//    entry.type = 'D';
//    update_entry(image, entry, offset);
//}


