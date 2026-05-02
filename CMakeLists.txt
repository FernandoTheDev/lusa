cmake_minimum_required(VERSION 3.28)

project(LusaLang VERSION 0.0.2 LANGUAGES C)

set(CMAKE_C_STANDARD 23)
set(CMAKE_C_STANDARD_REQUIRED True)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Release" CACHE STRING "Tipo de build" FORCE)
endif()

message(STATUS "LusaLang Build Type: ${CMAKE_BUILD_TYPE}")

set(SOURCES cli/main.c compiler/compiler.c compiler/lexer.c compiler/symtab.c compiler/codegen.c compiler/parser/core.c compiler/parser/expressions.c compiler/parser/statements.c lvm/lvm.c)

add_executable(lusa ${SOURCES})

target_include_directories(lusa PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)

if(MSVC)
    target_compile_options(lusa PRIVATE /W3)
else()
    target_compile_options(lusa PRIVATE -Wall -Wextra -Wno-unused-parameter)
endif()

if(CMAKE_BUILD_TYPE STREQUAL "Release")
    include(CheckIPOSupported)
    check_ipo_supported(RESULT ipo_supported)
    if(ipo_supported)
        set_property(TARGET lusa PROPERTY INTERPROCEDURAL_OPTIMIZATION TRUE)
    endif()
endif()

include(GNUInstallDirs)
install(TARGETS lusa DESTINATION ${CMAKE_INSTALL_BINDIR})

set(CPACK_PACKAGE_NAME "LusaLang")
set(CPACK_PACKAGE_VENDOR "nezumu")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "Compilador e VM da LusaLang")
set(CPACK_PACKAGE_VERSION ${PROJECT_VERSION})
set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_CURRENT_SOURCE_DIR}/LICENSE")

if(WIN32)
    set(CPACK_GENERATOR "NSIS")
    set(CPACK_NSIS_MODIFY_PATH ON)
    set(CPACK_PACKAGE_INSTALL_DIRECTORY "Lusa")
else()
    set(CPACK_GENERATOR "DEB;TGZ")
    set(CPACK_DEBIAN_PACKAGE_MAINTAINER "Nezumu Support")
    set(CPACK_DEBIAN_PACKAGE_SECTION "devel")
endif()

include(CPack)