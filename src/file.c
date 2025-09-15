#include "file.h"

#include <stdio.h>
#include <stdlib.h>


unsigned char* read_file_to_bytes(const char* filepath, size_t* file_size) {
    FILE* file = fopen(filepath, "rb");
    if (!file) return NULL;

    fseek(file, 0, SEEK_END);
    *file_size = ftell(file);
    fseek(file, 0, SEEK_SET);

    unsigned char* buffer = malloc(*file_size);
    if (!buffer) {
        fclose(file);
        return NULL;
    }

    fread(buffer, 1, *file_size, file);
    fclose(file);
    return buffer;
}


