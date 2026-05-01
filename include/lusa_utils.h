#pragma once

#include <stdio.h>

static inline int lusa_fopen(FILE** file, const char* path, const char* mode){
    #ifdef _WIN32
        return fopen_s(file, path, mode);
    #else
        *file = fopen(path, mode);
        return (*file != NULL) ? 0 : -1;
    #endif
}