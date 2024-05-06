#include <stdint.h>
#include <inttypes.h>


typedef struct __attribute__((__packed__)) {
    uint64_t file_size;
    Pointer pointer;
    char type;
    char name[256];
} DirEntry;

void export_file(Image image, DirEntry entry, FILE *out_file) {
    Pointer next_pointer_block = entry.pointer;
    if (next_pointer_block == 0) {
        fclose(out_file);
        return;
    }
    Pointer *pointer_block = malloc(image.meta.block_size);
    char *data_block = malloc(image.meta.block_size);
    uint64_t rem_bytes = entry.file_size;

    // enquanto ha pointer blocks
    while (next_pointer_block != 0) {
        // ler proximo pointer block
        pointer_block = read_block(image, next_pointer_block, pointer_block);
        Pointer *last_pointer = &pointer_block[image.meta.block_size/sizeof(Pointer) - 1];
        // para cada ponteiro direto
        for (Pointer *p = pointer_block; p != last_pointer; p++) {
            // se nulo, acabou arquivo
            if (*p == 0) {
                free(pointer_block);
                free(data_block);
                fclose(out_file);
                return;
            }
            // escrever bloco inteiro ou bytes restantes
            data_block = read_block(image, *p, data_block);
            if (rem_bytes >= image.meta.block_size) {
                fwrite(data_block, image.meta.block_size, 1, out_file);
                rem_bytes -= image.meta.block_size;
            } else {
                fwrite(data_block, rem_bytes, 1, out_file);
            }
        }
        // proximo ponteiro para pointer block
        next_pointer_block = *last_pointer;
    }
    free(pointer_block);
    free(data_block);
    fclose(out_file);
    return;
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

    if(!main_pointer) {
        fprintf(stderr, "Main pointer not allocd successfully!\n");
    }
    printf("Allocd at %"SCNu32" pointer\n", main_pointer);

    uint64_t read_bytes = 0;
    char *block = malloc(image.meta.block_size);
    fseek(image.file, main_pointer, SEEK_SET);

    do {
        read_bytes += fread(block, , image.meta.block_size, new_file);
        Pointer pointer = alloc_block(image);
        write_block(image, pointer, block, );
    } while(read_bytes < file_size);

    DirEntry new_entry = { file_size, main_pointer, 'F'};
    strcpy(new_entry.name, file_name);

    Pointer last_root_dir_pos = get_last_root_dir_pos(image);
    fseek(image.file, last_root_dir_pos, SEEK_SET);
    fwrite(&new_entry, sizeof(DirEntry), 1, image.file);

    printf("File written with %"SCNu64" bytes size at %"SCNu32" position in root dir \n", file_size, last_root_dir_pos);
    return 0;
}
