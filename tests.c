#include <stdio.h>
#include "utils.h"
#include "image.h"
#include "file.h"

int test_image_create_open(int *err) {
    FILE *file = fopen("./images/image_create.img", "wb+");
    const char *disk_name = "new_image";
    uint16_t block_size = 1024;
    uint32_t disk_size = 16;
    image_create(file, disk_name, block_size, disk_size);
    fclose(file);

    file = fopen("./images/image_create.img", "r");
    Image image = image_open(file);
    if (strncmp(disk_name, image.meta.disk_name, sizeof(image.meta.disk_name))) {
        ceprintf(RED, "☠ ERRO (image_create/image_open): disk_name não escrito corretamente\n");
        (*err)++;
    }

    if (block_size != image.meta.block_size) {
        ceprintf(RED, "☠ ERRO (image_create/image_open): block_size não escrito corretamente\n");
        (*err)++;
    }

    if (disk_size != image.meta.disk_size) {
        ceprintf(RED, "☠ ERRO (image_create/image_open): disk_size não escrito corretamente\n");
        (*err)++;
    }
    
    fseek(image.file, sizeof(image.meta), SEEK_SET);
    Pointer free_pointers[4];
    if (fread(free_pointers, sizeof(free_pointers), 1, image.file) != 1) {
        ceprintf(RED, "☠ ERRO (image_create/image_open): erro de leitura nos free pointers\n");
        (*err)++;
    }

    Pointer reference_pointers[4] = { 2, image.meta.disk_size-1, 0, 0 };
    if (memcmp(reference_pointers, free_pointers, sizeof(free_pointers))) {
        ceprintf(RED, "☠ ERRO (image_create/image_open): free pointers não escritos corretamente\n");
        (*err)++;
    }

    fseek(image.file, image.meta.block_size, SEEK_SET);
    Pointer p;
    if (fread(&p, sizeof(p), 1, image.file) != 1) {
        ceprintf(RED, "☠ ERRO (image_create/image_open): erro de leitura no bloco 1\n");
        (*err)++;
    }

    if (p != 0) {
        ceprintf(RED, "☠ ERRO (image_create/image_open): primeiro ponteiro do bloco 1 não escrito corretamente\n");
        (*err)++;
    }

    fseek(file, 0L, SEEK_END);
    uint64_t file_size = ftell(file);
    if(file_size != block_size * disk_size) {
        ceprintf(RED, "☠ ERRO (immage_create): imagem criada com tamanho errado. \n");
        (*err)++;
    }

    fclose(file);
}

int test_alloc_free_block(int *err) {
    FILE *file = fopen("./images/image_create.img", "wb+");
    const char *disk_name = "new_image";
    uint16_t block_size = 1024;
    uint32_t disk_size = 16;
    Image image = image_create(file, disk_name, block_size, disk_size);
    
    for (Pointer p = 2; p < 16; p++) {
        if (p != alloc_block(image)) {
            ceprintf(RED, "☠ ERRO (alloc_block/free_block): Comportamento inesperado na alocação\n");
            (*err)++;
        }
    }
    Pointer fail = alloc_block(image);
    if (fail != 0) {
        ceprintf(RED, "☠ ERRO (alloc_block/free_block): Não há blocos livres, mas alloc_block retornou não-nulo (%u)\n", fail);
        (*err)++;
    }
    for (Pointer p = 15; p >= 2; p--) {
        if (free_block(image, p)) {
            ceprintf(RED, "☠ ERRO (alloc_block/free_block): Bloco não foi liberado como esperado\n");
            (*err)++;
        }
    }
    for (Pointer p = 2; p < 16; p++) {
        if (p != alloc_block(image)) {
            ceprintf(RED, "☠ ERRO (alloc_block/free_block): Comportamento inesperado na alocação após liberamento\n");
            (*err)++;
            break;
        }
    }
}

void test_export_file(int *err) {
    // criando imagem em branco
    FILE *file = fopen("./images/test_export_file", "w+");
    uint16_t block_size = 1024; 
    uint32_t disk_size = 16;
    Image image = image_create(file, "export file disk", block_size, disk_size);
    
    // inserindo um arquivo exemplo
    const char *data = "#include <stdio.h>\n\nint main(int argc, char **argv) {\n    printf(\"Hello, World\\n\");\n    return 0;\n}\n";
    size_t data_size = strlen(data);
    const char *filename = "./images/hello.c";
    FILE *out_file = fopen(filename, "wb");
    DirEntry entry = {
        .file_size = data_size,
        .type = 'F',
        .pointer = 2,
    };
    strncpy(entry.name, filename, sizeof(entry.name));
    Pointer pointer_block[2] = { 3, 0 };

    fseek(file, block_size, SEEK_SET);
    fwrite(&entry, sizeof(entry), 1, file);
    fseek(file, block_size*2, SEEK_SET);
    fwrite(pointer_block, sizeof(pointer_block), 1, file);
    fseek(file, block_size*3, SEEK_SET);
    fwrite(data, data_size, 1, file);

    // exportando arquivo
    export_file(image, entry, out_file);
    fclose(out_file);
    char read_data[128];
    FILE *read_file = fopen(filename, "r");
    if (read_file == NULL) {
        ceprintf(RED, "☠ ERRO (export_file): Arquivo não foi escrito\n");
        (*err)++;
    }
    fread(read_data, data_size, 1, read_file);
    read_data[data_size];
    if (memcmp(data, read_data, data_size)) {
        ceprintf(RED, "☠ ERRO (export_file): Conteúdo do arquivo não foi escrito corretamente\n");
        (*err)++;
    }
}

int main() {
    int err = 0;
    test_image_create_open(&err);
    test_alloc_free_block(&err);
    test_export_file(&err);

    const char *color = (err == 0) ? GREEN : RED;
    ceprintf(color, "%i erros\n", err);

    return err;
}

