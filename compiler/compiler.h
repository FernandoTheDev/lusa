#pragma once
#include <stdint.h>
#include <string.h>
#include "lusa_utils.h"

LusaModule *compile_to_module(const char *source);

int build_executable(const char *current_exe, const char *bc_file, const char *out_exe);

int compile(const char *in_filepath, const char *out_filepath);
