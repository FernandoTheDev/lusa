#pragma once

#include <stddef.h>
#include <string.h>

// nao precisa de uma função propria
static inline void lusa_strcpy(char* dest, size_t dest_size, const char* src){
    if (dest_size == 0) return;

    #ifdef _WIN32
        strcpy_s(dest, dest_size, src);
    #else
        size_t i = 0;
        while (i < dest_size - 1 && src[i] != '\0'){
            dest[i] = src[i];
            i++;
        }
        dest[i] = '\0';
    #endif
}

static inline void lusa_strcat(char* dest, size_t dest_size, const char* src){
    #ifdef _WIN32
    strcat_s(dest, dest_size, src);
    #else
    strncat(dest, src, dest_size);
    #endif
}
