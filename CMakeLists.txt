cmake_minimum_required(VERSION 3.30)

project(LusaLang VERSION 0.0.1 LANGUAGES C)

set(CMAKE_C_STANDARD 23)
set(CMAKE_C_STANDARD_REQUIRED True)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Debug" CACHE STRING "Tipo de build: Debug (Dev) ou Release (Prod)" FORCE)
endif()

message(STATUS "LusaLang Build Type: ${CMAKE_BUILD_TYPE}")

set(SOURCES cli/main.c compiler/compiler.c compiler/lexer.c compiler/symtab.c compiler/codegen.c compiler/parser/core.c compiler/parser/expressions.c compiler/parser/statements.c vm/vm.c)

add_executable(lusa ${SOURCES})

if(MSVC)
    target_compile_options(lusa PRIVATE /W3)
else()
    target_compile_options(lusa PRIVATE -Wall -Wextra -Wno-unused-parameter)
endif()

if(CMAKE_BUILD_TYPE STREQUAL "Release")
    include(CheckIPOSupported)
    check_ipo_supported(RESULT ipo_supported)
    if(ipo_supported)
        message(STATUS "LTO (Link Time Optimization) ativado para modo Produção!")
        set_property(TARGET lusa PROPERTY INTERPROCEDURAL_OPTIMIZATION TRUE)
    endif()
endif()

install(TARGETS lusa DESTINATION bin)

set(CPACK_PACKAGE_NAME "LusaLang")
set(CPACK_PACKAGE_VENDOR "nezumu")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "Compilador e VM da LusaLang")
set(CPACK_PACKAGE_VERSION "0.0.1")
set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_CURRENT_SOURCE_DIR}/LICENSE")
set(CPACK_GENERATOR "NSIS")

set(CPACK_NSIS_MODIFY_PATH ON)

include(CPack)