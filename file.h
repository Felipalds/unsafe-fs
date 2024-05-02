#include <stdint.h>
#include <inttypes.h>


typedef struct DirEntry {
    uint16_t entry_size;
    uint16_t name_size;
    char entry_type;
    uint64_t file_size;
    Pointer pointer;
} DirEntry;

void export_file(Image image, DirEntry entry, const char *filename) {
    FILE *out_file = fopen(filename, "w+");
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

void list_root_dir(Image image) {
    int c = 0;
    DirEntry dir_entry;

    fseek(image.file, image.meta.block_size, SEEK_SET);

    fread(&dir_entry, sizeof(dir_entry), 1, image.file);
    char file_name[dir_entry.name_size];
    fread(&file_name, dir_entry.name_size, 1, image.file);

    printf("Name 1: %s", file_name);
}

int import_file (Image image, char path[256], char file_name[256]) {
    FILE* file = fopen(path, "r");
    if(file == NULL) {
        printf("File couldnt be opened\n");
    }

    // verificar em quantos blocos o arquivo vai caber
    // pegar os blocos livres
    // escrever no img
    // criar a entry no root dir

    fseek(file, 0L, SEEK_END);
    uint64_t file_size = ftell(file);
    Pointer main_pointer;

    if(file_size <= image.meta.block_size) {
        // file cabe em um bloco
        main_pointer = alloc_block(image);


    } else {
        // file cabe em mais de um bloco
        // escrever o file usando em consideracao o tamanho do pointer
    }

    DirEntry new_entry = { 0, sizeof(file_name), 'f', file_size, main_pointer};


    printf("File has %"SCNu64" bytes size \n", file_size);
    return 0;
}
