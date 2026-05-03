#pragma once

#include <stdio.h>

#define LUSA_MAGIC 0x4C555341 // nunca é usado

typedef struct
{
    uint32_t *code;
    uint32_t code_size;
    char (*strings)[1000]; // terrivel
    uint16_t string_count;
    double *floats;
    uint16_t float_count;
} LusaModule;

static inline int lusa_fopen(FILE **file, const char *path, const char *mode)
{
#ifdef _WIN32
    return fopen_s(file, path, mode);
#else
    *file = fopen(path, mode);
    return (*file != NULL) ? 0 : -1;
#endif
}
