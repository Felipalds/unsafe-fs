#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <inttypes.h>
#include "image.h"
#include "file.h"
#include "utils.h"

void usage() {
    fprintf(stderr, "USAGE\n");
    fprintf(stderr, "    --create /path/to/image DISK_NAME     BLOCK_SIZE DISK_SIZE\n");
    fprintf(stderr, "    --import /path/to/image /path/to/file FILE_NAME\n");
    fprintf(stderr, "    --export /path/to/image /path/to/file FILE_NAME\n");
    fprintf(stderr, "    --delete /path/to/image FILE_NAME\n");
    fprintf(stderr, "    --list   /path/to/image\n");
}

int main(int argc, const char **argv) {
    if (argc == 6 && !strcmp(argv[1], "--create")) {
        const char *disk_name = argv[3];
        uint16_t block_size;
        if (sscanf(argv[4], "%"SCNu16, &block_size) != 1) {
            usage();
            return 1;
        }
        if (block_size < sizeof(MetaBlock) + 4*sizeof(Interval)) {
            ceprintf(RED, "Tamanho do bloco %"PRIu16" é muito pequeno\n", block_size);
            return 1;
        }
        uint32_t disk_size;
        if (sscanf(argv[5], "%"SCNu32, &disk_size) != 1) {
            usage();
            return 1;
        }
        if (disk_size < 2) {
            ceprintf(RED, "Tamanho do disco %"PRIu32" é muito pequeno\n", disk_size);
            return 1;
        }
        FILE *file = fopen(argv[2], "wbx");
        if (file == NULL) {
            ceprintf(RED, "Cannot create file '%s'\n", argv[2]);
            return 1;
        }
        Image image = image_create(file, disk_name, block_size, disk_size);
        fclose(file);
    } else if (argc == 5 && !strcmp(argv[1], "--import")) {
        FILE *file = fopen(argv[2], "rb+");
        if (file == NULL) {
            ceprintf(RED, "Cannot open image '%s'\n", argv[2]);
            return 1;
        }
        Image image = image_open(file);
        FILE *in_file = fopen(argv[3], "rb");
        if (in_file == NULL) {
            ceprintf(RED, "Cannot open file '%s'\n", argv[3]);
            return 1;
        }
        const char *file_name = argv[4];
        import_file(image, in_file, file_name);
        fclose(in_file);
        fclose(file);
    } else if (argc == 5 && !strcmp(argv[1], "--export")) {
        // EXPORT
        FILE *file = fopen(argv[2], "rb");
        if (file == NULL) {
            ceprintf(RED, "Cannot open image '%s'\n", argv[2]);
            return 1;
        }
        Image image = image_open(file);
        FILE *out_file = fopen(argv[3], "wbx");
        if (out_file == NULL) {
            ceprintf(RED, "Cannot create file '%s'\n", argv[3]);
            return 1;
        }
        // encontrar direntry do arquivo
        // criar uma função find(filename) -> DirEntry?
        /*
         * int err;
         * DirEntry entry = find(image, argv[4], &err);
         * if (err) {
         *     ceprintf(RED, "Cannot find file '%s' in image\n", argv[4]);
         *     return 1;
         * }
         * export_file(image, entry, out_file)
         */
        fclose(file);
        fclose(out_file);
    } else if (argc == 4 && !strcmp(argv[1], "--delete")) {
        FILE *file = fopen(argv[2], "rb+");
        if (file == NULL) {
            ceprintf(RED, "Cannot open image '%s'\n", argv[2]);
            return 1;
        }
        Image image = image_open(file);
        /*
         * delete_file(image, filename)
         */
        fclose(file);
    } else if (argc == 3 && !strcmp(argv[1], "--list")) {
        FILE *file = fopen(argv[2], "rb+");
        if (file == NULL) {
            ceprintf(RED, "Cannot open image '%s'\n", argv[2]);
            return 1;
        }
        Image image = image_open(file);
        list_root_dir(image);
        fclose(file);
    } else {
        usage();
        return 1;
    }
}

