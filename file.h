#include <stdint.h>


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

void list_root_dir() {}
