#include <stdint.h>
#include <inttypes.h>


typedef struct __attribute__((__packed__)) {
    uint16_t entry_size;
    uint16_t name_size;
    char entry_type;
    uint64_t file_size;
    Pointer pointer;
} DirEntry;

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

void list_root_dir(Image image) {
    int c = 0;
    DirEntry dir_entry;

    fseek(image.file, image.meta.block_size, SEEK_SET);

    fread(&dir_entry, sizeof(dir_entry), 1, image.file);
    char file_name[dir_entry.name_size];
    fread(&file_name, dir_entry.name_size, 1, image.file);

    printf("Name 1: %s", file_name);
}


Pointer get_last_root_dir_pos ( Image image ) {
    fseek(image.file, image.meta.block_size, SEEK_SET);

    DirEntry entry;

    fread(&entry, sizeof(DirEntry), 1, image.file);
    while(entry.entry_size > 0) {
        fseek(image.file, entry.name_size, SEEK_CUR);
        fread(&entry, sizeof(DirEntry), 1, image.file);
    }
    return ftell(image.file);
}

int import_file (Image image, const char path[256], const char file_name[256]) {
    FILE* importing_file = fopen(path, "r");
    if(importing_file == NULL) {
        printf("File couldnt be opened\n");
    }


    // verificar em quantos blocos o arquivo vai caber
    // pegar os blocos livres
    // escrever no img
    // criar a entry no root dir

    fseek(importing_file, 0L, SEEK_END);
    uint64_t file_size = ftell(importing_file);
    printf("File size is %"SCNu64" bytes\n", file_size);
    Pointer main_pointer = alloc_block(image);
    printf("Allocd at %"SCNu32" pointer\n", main_pointer);

    if(file_size <= image.meta.block_size) {
        // importing_file cabe em um bloco
        fseek(image.file, main_pointer, SEEK_SET);
        char ch = fgetc(importing_file);
        while (ch != EOF) {
            fputc(ch, image.file);
            ch = fgetc(importing_file);
        }
        fputc(EOF, image.file);
    } else {
        // importing_file cabe em mais de um bloco
        // escrever o importing_file usando em consideracao o tamanho do pointer
        int num_blocks = file_size / (image.meta.block_size - sizeof(Pointer));

    }

    DirEntry new_entry = { strlen(file_name) + file_size + 1, strlen(file_name), 'f', file_size, main_pointer};
    Pointer last_root_dir_pos = get_last_root_dir_pos(image);
    fseek(image.file, last_root_dir_pos, SEEK_SET);
    fwrite(&new_entry, sizeof(new_entry), 1, image.file);
    fwrite(&file_name, strlen(file_name) + 1, 1, image.file);

    printf("File written with %"SCNu64" bytes size at %"SCNu32" position \n", file_size, last_root_dir_pos);
    return 0;
}
