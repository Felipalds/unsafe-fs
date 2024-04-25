#include <string.h>


typedef uint32_t Pointer;

typedef struct __attribute__((__packed__)) {
    char disk_name[16];
    uint16_t block_size;
    uint32_t disk_size;
} MetaBlock;

typedef struct __attribute__((__packed__)) {
    Pointer begin;
    Pointer end;
} Interval;

typedef struct Image {
    MetaBlock meta;
    FILE* file;
} Image;


Image image_create(FILE *file, const char *disk_name, uint16_t block_size, uint32_t disk_size) {
    // preparando e escrevendo MetaBlock
    MetaBlock meta;
    meta.block_size = block_size;
    meta.disk_size = disk_size;
    memset(meta.disk_name, 0, sizeof(meta.disk_name));
    for (int i = 0; disk_name[i] != '\0' && i < sizeof(meta.disk_name); i++) {
        meta.disk_name[i] = disk_name[i];
    }
    fseek(file, 0, SEEK_SET);
    fwrite(&meta, sizeof(meta), 1, file);

    // primeiros ponteiros livres
    Pointer free_pointers[] = { 2, disk_size, 0, 0 };
    fwrite(free_pointers, sizeof(free_pointers), 1, file);
    
    // root dir vazio
    Pointer null_block = 0;
    fseek(file, block_size, SEEK_SET);
    fwrite(&null_block, sizeof(null_block), 1, file);

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

void list_free_blocks(Image image) {
    fseek(image.file, sizeof(image.meta), SEEK_SET);
    Interval interval;
    for (;;) {
        if (ftell(image.file) >= image.meta.block_size) {
            break;
        }
        if (fread(&interval, sizeof(interval), 1, image.file) != 1) {
            break;
        }
        if (interval.begin == 0) {
            break;
        }
        printf("%u .. %u\n", interval.begin, interval.end);
    }
    printf("\n");
}

Pointer alloc_block(Image image) {
    fseek(image.file, sizeof(image.meta), SEEK_SET);
    Interval interval;
    for (;;) {
        if (ftell(image.file) >= image.meta.block_size) {
            return 0;
        }
        if (fread(&interval, sizeof(interval), 1, image.file) != 1) {
            return 0;
        }
        if (interval.begin == 0) {
            return 0;
        }
        if (interval.begin > interval.end) {
            continue;
        }
        Pointer alloc = interval.begin;
        interval.begin++;
        fseek(image.file, -sizeof(interval), SEEK_CUR);
        fwrite(&interval, sizeof(interval), 1, image.file);
        return alloc;
    }
}

void free_block(Image image, Pointer block) {
    fseek(image.file, sizeof(image.meta), SEEK_SET);
    Interval interval;
    for (;;) {
        if (ftell(image.file) >= image.meta.block_size) {
            // erro: deve comprimir gerenciamento de blocos livres
            return;
        }
        fread(&interval, sizeof(interval), 1, image.file);
        if (interval.begin <= block && block <= interval.end) {
            return;
        }
        if (interval.begin == block+1) {
            interval.begin--;
            break;
        } else if (interval.end == block-1) {
            interval.end++;
            break;
        } else if (interval.begin == 0) {
            interval.begin = block;
            interval.end = block;
            break;
        }
    }
    fseek(image.file, -sizeof(interval), SEEK_CUR);
    fwrite(&interval, sizeof(interval), 1, image.file);
}

