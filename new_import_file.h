void find_root_dir_gap(Image image, Pointer *p_block, size_t *p_entry) {
    bool found_empty = false;
    DirEntry *data_block = NULL;
    Pointer *pointer_block = read_block(image, 1, NULL);
    size_t block = 0;
    size_t entry = 0;
    while (!found_empty && pointer_block[block] && block < image.meta.block_size / sizeof(Pointer)) {
        data_block = read_block(image, pointer_block[block], data_block);
        entry = 0; 
        while (entry < image.meta.block_size / sizeof(DirEntry)) {
            if (data_block[entry].type == 'D') {
                found_empty = true;
                break;
            }
            entry++;
        }
        block++;
    }
    // temos que alocar novo bloco no root dir
    if (!found_empty) {
        entry = 0;
        if (block >= image.meta.block_size / sizeof(Pointer)) {
            exit(1);
        }
        Pointer new_block = alloc_block(image);
        if (!new_block) {
            exit(1);
        }
        pointer_block[block] = new_block;
        // preencher com um direntry 'D'
        fseek(image.file, new_block*image.meta.block_size, SEEK_SET);
        for (size_t i = 0; i < image.meta.block_size / sizeof(DirEntry); i++) {
            fwrite(&entry, sizeof(entry), 1, image.file);
        }
    }
    if (p_block) {
        *p_block = block;
    }
    if (p_entry) {
        *p_entry = entry;
    }
    free(data_block);
    free(pointer_block);
}

// retorna 0 se OK, 1 se erro
int import_file_new(Image image, const char *name, FILE *in_file) {
    Pointer block;
    size_t entry;
    find_root_dir_gap(image, &block, &entry);

    // tamanho do arquivo
    fseek(in_file, 0, SEEK_END);
    uint64_t file_size = ftell(in_file);
    printf("Importing a file with %d\n", file_size);

    Pointer pointer_block = 0;
    if (file_size != 0) {
        // aloca novo pointer block
        pointer_block = alloc_block(image);
        if (!pointer_block) {
            exit(1);
        }
    }

    // prepara entry
    DirEntry new_entry = {
        .file_size = file_size,
        .type = 'F',
        .pointer = pointer_block,
        .name = {0}
    };
    strncpy(new_entry.name, name, sizeof(new_entry.name));
    printf("New file: %s\n", new_entry.name);

    fseek(image.file, block*image.meta.block_size + entry*sizeof(DirEntry), SEEK_SET);
    fwrite(&new_entry, sizeof(new_entry), 1, image.file);

    size_t i = 0;
    Pointer *pointer_block_data = malloc(image.meta.block_size);
    char *data_block = malloc(image.meta.block_size);
    uint64_t read_bytes = 0;
    while (read_bytes < file_size && i < image.meta.block_size / sizeof(Pointer)) {
        size_t fread_bytes = fread(data_block, 1, image.meta.block_size, in_file);
        if (fread_bytes == 0) {
            break;
        }
        read_bytes += fread_bytes;
        Pointer data_block = alloc_block(image);
        pointer_block_data[i] = data_block;
        i++;
    }
    write_block(image, pointer_block, (char*)pointer_block_data, image.meta.block_size);

    free(data_block);
    free(pointer_block_data);
}
