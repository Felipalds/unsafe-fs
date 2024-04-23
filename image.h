#include <string.h>


typedef uint32_t Pointer;

typedef struct MetaBlock {
    char disk_name[16];
    uint16_t block_size;
    uint32_t disk_size;
} MetaBlock;

typedef struct Image {
    MetaBlock meta;
    FILE* file;
} Image;

typedef struct Interval {
    Pointer begin_pointer;
    Pointer end_pointer;
} Interval;

Image image_create(FILE *file, const char *disk_name, uint16_t block_size, uint32_t disk_size) {
    // preparando e escrevendo MetaBlock
    MetaBlock meta;
    meta.block_size = block_size;
    meta.disk_size = disk_size;
    memset(meta.disk_name, 0, sizeof(meta.disk_name));
    for (int i = 0; disk_name[i] != '\0' && i < 16; i++) {
        meta.disk_name[i] = disk_name[i];
    }
    fseek(file, 0, SEEK_SET);
    fwrite(&meta, sizeof(meta), 1, file);

    // primeiros ponteiros livres
    Pointer free_pointers[4] = { 2, disk_size, 0, 0 };
    fseek(file, sizeof(meta), SEEK_SET);
    fwrite(free_pointers, sizeof(free_pointers), 1, file);
    
    // root dir vazio
    Pointer nullptr = 0;
    fseek(file, block_size, SEEK_SET);
    fwrite(&nullptr, sizeof(Pointer), 1, file);

    return (Image) {
        .meta = meta,
        .file = file
    };
}

Image image_open(FILE *file) {
    MetaBlock meta;
    fseek(file, 0, SEEK_SET);
    fread(&meta, sizeof(meta), 1, file); 
    // TODO checar erro de leitura
    
    return (Image) {
        .meta = meta,
        .file = file
    };
}

Pointer alloc_block(Image image) {
    return 0;
}

void free_block(Image image, Pointer block) {
}

