#include <stdint.h>
#include <inttypes.h>

typedef struct __attribute__((__packed__)) {
    uint64_t file_size;
    Pointer pointer;
    char type;
    char name[256];
} DirEntry;

void print_entry(DirEntry entry) {
    printf("%s --- ", entry.name);
    printf("%"SCNu64" bytes\n", entry.file_size);
}

DirEntry find_by_name(Image image, const char *name, int* err, Pointer* pointer,  int* offset) {
    DirEntry found;
    printf("finding file!!!!!!!!!!\n");
    Pointer *pointer_block = read_block(image, 1, NULL);
    Pointer *last_pointer = &pointer_block[image.meta.block_size/sizeof(Pointer)];
    DirEntry *block = NULL;
    bool find = false;
    uint16_t rem_entries = image.meta.entries;
    for (Pointer *cur_pointer = pointer_block; cur_pointer != last_pointer && !find; cur_pointer++) {
        block = read_block(image, *cur_pointer, block);
        DirEntry *last_entry = &block[image.meta.block_size / sizeof(DirEntry)];
        for (DirEntry *entry = block; entry != last_entry; entry++) {
            printf("dir_entry\n");
            if (rem_entries == 0) {
                free(block);
                free(pointer_block);
                fprintf(stderr, "file does not exist");
                exit(1);
            }
            if (entry->type != 'D' && !strcmp(entry->name, name)) {
                found = *entry;
                find = true;
                break;
            }
            rem_entries--;
            print_entry(*entry);
        }
    }

    return found;
}

int delete_entry(Image *image, const char *entry_name) {
    int err = 0;
    Pointer entry_block;
    int entry_offset;
    DirEntry entry = find_by_name(*image, entry_name, &err, &entry_block, &entry_offset);
    if (entry.type == 'D') {
        return 1;
    }
    // escreve novo entry como tipo 'D'
    uint64_t offset = entry_block*image->meta.block_size + entry_offset*sizeof(DirEntry);
    entry.type = 'D';
    fseek(image->file, offset, SEEK_SET);
    fwrite(&entry, sizeof(entry), 1, image->file);
    // libera todos os blocos do pointer block
    Pointer *pointer_block = read_block(*image, entry.pointer, NULL);
    Pointer *end = &pointer_block[image->meta.block_size / sizeof(Pointer)];
    for (Pointer *pointer = pointer_block; *pointer && pointer < end; pointer++) {
        free_block(*image, *pointer);
    }
    // e libera próprio pointer block
    free_block(*image, entry.pointer);
    free(pointer_block);
    return 0;
}

void export_file(Image image, DirEntry entry, FILE *out_file) {
    if (entry.file_size == 0) {
        return;
    }
    uint64_t rem_bytes = entry.file_size;
    Pointer *pointer_block = read_block(image, entry.pointer, NULL);
    Pointer *end = &pointer_block[image.meta.block_size/sizeof(Pointer)];
    char *data_block = NULL; 
    for (Pointer *cur = pointer_block; cur != end; cur++) {
        data_block = read_block(image, *cur, data_block);
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

void write_entry(Image image, DirEntry entry, Pointer block, long offset) {
    long byte_offset = block*image.meta.block_size + offset*sizeof(DirEntry);
    fseek(image.file, byte_offset, SEEK_SET);
    fwrite(&entry, sizeof(entry), 1, image.file);
}

int alloc_entry(Image *image, Pointer *p_block, size_t *p_offset) {
    Pointer *root_dir_pointer_block = read_block(*image, 1, NULL);
    size_t max_entries = image->meta.block_size / sizeof(DirEntry);
    size_t block_index = image->meta.entries / max_entries;
    Pointer which_block = root_dir_pointer_block[block_index];
    size_t which_entry = image->meta.entries % max_entries;

    if (which_block == 0) {
        which_block = alloc_block(*image);
        if (!which_block) {
            free(root_dir_pointer_block);
            return 1;
        }
        fseek(image->file, image->meta.block_size + block_index*sizeof(Pointer), SEEK_SET);
        fwrite(&which_block, sizeof(which_block), 1, image->file);
    }

    *p_block = which_block;
    *p_offset = which_entry;
    image->meta.entries++;
    free(root_dir_pointer_block);
    return 0;
}

void list_root_dir(Image image) {
    Pointer *pointer_block = read_block(image, 1, NULL);
    Pointer *last_pointer = &pointer_block[image.meta.block_size/sizeof(Pointer)];
    DirEntry *block = NULL;
    uint16_t rem_entries = image.meta.entries;
    for (Pointer *cur_pointer = pointer_block; cur_pointer != last_pointer; cur_pointer++) {
        block = read_block(image, *cur_pointer, block);
        DirEntry *last_entry = &block[image.meta.block_size / sizeof(DirEntry)];
        for (DirEntry *entry = block; entry != last_entry; entry++) {
            if (rem_entries == 0) {
                free(block);
                free(pointer_block);
                return;
            }
            if (entry->type == 'D') {
                continue;
            }
            rem_entries--;
            print_entry(*entry);
        }
    }
}


int import_file(Image *image, FILE* new_file, const char *file_name) {
    fseek(new_file, 0L, SEEK_END);
    uint64_t file_size = ftell(new_file);
    printf("File size is %"SCNu64" bytes\n", file_size);
    Pointer main_pointer = alloc_block(*image);
    Pointer *pointer_block = calloc(1, image->meta.block_size);
    fseek(new_file, 0, SEEK_SET);

    if (!main_pointer) {
        fprintf(stderr, "Main pointer not allocd successfully!\n");
        return 1;
    }
    printf("Allocd at %"SCNu32" pointer\n", main_pointer);

    char *data_block = malloc(image->meta.block_size);
    fseek(image->file, main_pointer, SEEK_SET);
    int offset = 1;
    uint64_t read_bytes = 0;

    do {
        read_bytes += fread(data_block, 1, image->meta.block_size, new_file);
        Pointer pointer = alloc_block(*image);
        write_block(*image, pointer, data_block, image->meta.block_size);
        pointer_block[offset++] = pointer;
        offset++;
    } while (read_bytes < file_size);
    write_block(*image, main_pointer, (const char*)pointer_block, image->meta.block_size);

    DirEntry new_entry = {
        .file_size = file_size,
        .pointer = main_pointer,
        .type = 'F',
        .name = { 0 },
    };
    strncpy(new_entry.name, file_name, sizeof(new_entry.name));

    Pointer dir_block;
    size_t dir_offset;
    if (alloc_entry(image, &dir_block, &dir_offset)) {
        free(data_block);
        printf("nao pode alocar entry\n");
        return 1;
    }
    write_entry(*image, new_entry, dir_block, dir_offset);

    printf("File written with %"SCNu64" bytes size at block %"SCNu32" and position %zu in root dir \n", file_size, dir_block, dir_offset);
    return 0;
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
