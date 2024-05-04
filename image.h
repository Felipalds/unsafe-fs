#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <inttypes.h> // for SCNu16 and SCNu32

typedef uint32_t Pointer;

typedef struct __attribute__((__packed__)) {
    char disk_name[16];
    uint16_t block_size;
    uint32_t disk_size;
} MetaBlock;

void view_meta(MetaBlock meta) {
    printf("DISK NAME: %s\n", meta.disk_name);
    printf("BLOCK SIZE: %"SCNu16"\n", meta.block_size);
    printf("DISK SIZE: %"SCNu32"\n", meta.disk_size);
}

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
    strncpy(meta.disk_name, disk_name, sizeof(meta.disk_name));
    fseek(file, 0, SEEK_SET);
    fwrite(&meta, sizeof(meta), 1, file);

    // primeiros ponteiros livres
    Pointer free_pointers[] = { 2, disk_size-1, 0, 0 };
    fwrite(free_pointers, sizeof(free_pointers), 1, file);

    // root dir vazio
    Pointer null_block = 0;
    fseek(file, block_size, SEEK_SET);
    fwrite(&null_block, sizeof(null_block), 1, file);

    // updating the file size
    fseek(file, block_size * disk_size - 1, SEEK_SET);
    fwrite("", 1, 1, file);

    return (Image) {
        .meta = meta,
        .file = file
    };
}

Image image_open(FILE *file) {
    fseek(file, 0, SEEK_SET);
    MetaBlock meta;
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

// retorna 0 se não encontrar blocos livres
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

// retorna 0 se OK, 1 se bloco não foi liberado
int free_block(Image image, Pointer block) {
    // não liberar meta e root
    if (block <= 1) {
        return 1;
    }
    fseek(image.file, sizeof(image.meta), SEEK_SET);
    Interval interval;
    for (;;) {
        if (ftell(image.file) >= image.meta.block_size) {
            // erro: deve desfragmentar gerenciamento de blocos livres
            return 1;
        }
        fread(&interval, sizeof(interval), 1, image.file);
        if (interval.begin <= block && block <= interval.end) {
            // bloco ja estava livre
            return 1;
        }
        if (interval.begin == block+1) {
            interval.begin--;
            break;
        }
        if (interval.end == block-1) {
            interval.end++;
            break;
        }
        if (interval.begin == 0) {
            interval.begin = block;
            interval.end = block;
            break;
        }
    }
    fseek(image.file, -sizeof(interval), SEEK_CUR);
    fwrite(&interval, sizeof(interval), 1, image.file);
    return 0;
}

//  para alocar novo bloco:
//      char *block = read_block(image, pointer, NULL);
//  para reusar um buffer ja alocado:
//      block = read_block(image, pointer, bloco);
void *read_block(Image image, Pointer pointer, void *block) {
    if (block == NULL) {
        block = malloc(image.meta.block_size);
    }
    fseek(image.file, pointer*image.meta.block_size, SEEK_SET);
    fread(block, image.meta.block_size, 1, image.file);
    return block;
}

// escreve em _qualquer_ bloco, inclusive Meta (0)
// retorna 0 se OK, 1 se não escreveu
int write_block(Image image, Pointer pointer, const char *data, size_t size) {
    if (size > image.meta.block_size) {
        size = image.meta.block_size;
    }
    fseek(image.file, pointer*image.meta.block_size, SEEK_SET);
    if (fwrite(data, size, 1, image.file) != 1) {
        return 1;
    }
    return 0;
}

typedef struct {
    Image image;
    Pointer next_pointer_block;
    Pointer *pointer_block; // free(pointer_block) pls
    Pointer *last_pointer;
    Pointer *current_pointer;
} BlockIter;

// como iterar blocos de um arquivo
//  BlockIter it = block_iter(image, entry.pointer);
//  void *buf = next_block(&it, NULL); // NULL ou algum buffer ja alocado
//  for (TIPO *block = buf; block != NULL; block = next_block(&it, block)) {
//      /* faz alguma coisa com block */
//  }
//  free(it.pointer_block);
//  free(buf);
BlockIter block_iter(Image image, Pointer next_pointer_block) {
    Pointer *pointer_block = NULL;
    return (BlockIter) {
        .image = image,
        .next_pointer_block = next_pointer_block,
        .pointer_block = NULL,
        .last_pointer = NULL,
        .current_pointer = NULL,
    };
}

void *next_block(BlockIter *it, void *data_block) {
    if (it->next_pointer_block == 0) {
        return NULL;
    }
    if (it->current_pointer == it->last_pointer) {
        if (it->next_pointer_block == 0) {
            return NULL;
        }
        it->pointer_block = read_block(it->image, it->next_pointer_block, it->pointer_block);
        it->last_pointer = &it->pointer_block[it->image.meta.block_size / sizeof(Pointer) - 1];
        it->next_pointer_block = *it->last_pointer;
        it->current_pointer = it->pointer_block;
    }
    if (*it->current_pointer == 0) {
        return NULL;
    }
    data_block = read_block(it->image, *it->current_pointer, data_block);
    it->current_pointer++;
    return data_block;
}

