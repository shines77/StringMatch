CMAKE_MINIMUM_REQUIRED(VERSION 2.8)

project(StringMatch)

set(CMAKE_VERBOSE_MAKEFILE ON)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/Modules/")

include("${PROJECT_SOURCE_DIR}/cmake/detect_cpu_architectures.cmake")

enable_language(ASM_NASM)

if (CMAKE_CL_64)
    set(Platform x64)
else(CMAKE_CL_64)
    set(Platform x86)
endif(CMAKE_CL_64)

if (NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

message("------------ Options -------------")
message("  CMAKE_BUILD_TYPE         : ${CMAKE_BUILD_TYPE}")
message("  Platform                 : ${Platform}")
message("  CMAKE_CPU_ARCHITECTURES  : ${CMAKE_CPU_ARCHITECTURES}")
message("----------------------------------")

message("-------------- Env ---------------")
message("  CMAKE_SOURCE_DIR: ${CMAKE_SOURCE_DIR}")
message("  CMAKE_BINARY_DIR: ${CMAKE_BINARY_DIR}")
message("  PROJECT_SOURCE_DIR: ${PROJECT_SOURCE_DIR}")
message("  PROJECT_BINARY_DIR: ${PROJECT_BINARY_DIR}")
message("----------------------------------")

if (CMAKE_BUILD_TYPE STREQUAL Debug)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -D_DEBUG")
else()
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DNDEBUG")
endif()

if (NOT MSVC)
    ## -Wall -Werror -Wextra -Wno-format -Wno-unused-function
    set(CMAKE_CXX_FLAGS_DEFAULT "${CMAKE_CXX_FLAGS} -std=c++11 -march=native -mmmx -msse -msse2 -msse3 -mssse3 -msse4 -msse4a -msse4.1 -msse4.2 -Wall -Wno-unused-function -Wno-deprecated-declarations -Wno-unused-variable -fPIC -U__STRICT_ANSI__")
    ## add_compile_options(-D__SSE3__ -D__SSE4A__ -D__SSE4_1__ -D__SSE4_2__)
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_DEFAULT} -O3 -DNDEBUG")
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEFAULT} -g -pg -D_DEBUG")
endif()

if (WIN32)
    add_compile_options(-D_WIN32_WINNT=0x0601 -D_CRT_SECURE_NO_WARNINGS)
    set(EXTRA_LIBS ${EXTRA_LIBS} ws2_32 mswsock)
endif()

if (UNIX)
    set(EXTRA_LIBS ${EXTRA_LIBS} pthread)
else()
    set(EXTRA_LIBS ${EXTRA_LIBS} ${CMAKE_THREAD_LIBS_INIT})
endif()

include_directories(include)
include_directories(src)
include_directories(src/main)
include_directories(src/test)
include_directories(src/unittest)

set(SOURCE_FILES
    src/main/main.cpp
    )

add_custom_target(debug
    COMMAND ${CMAKE_COMMAND} -DCMAKE_BUILD_TYPE=Debug ${CMAKE_SOURCE_DIR}
    COMMAND ${CMAKE_COMMAND} --build ${CMAKE_BINARY_DIR} --target all
    COMMENT "Switch CMAKE_BUILD_TYPE to Debug"
)

add_custom_target(release
    COMMAND ${CMAKE_COMMAND} -DCMAKE_BUILD_TYPE=Release ${CMAKE_SOURCE_DIR}
    COMMAND ${CMAKE_COMMAND} --build ${CMAKE_BINARY_DIR} --target all
    COMMENT "Switch CMAKE_BUILD_TYPE to Release"
)

if ("amd64" STREQUAL "${CMAKE_CPU_ARCHITECTURES}" OR "x64" STREQUAL "${CMAKE_CPU_ARCHITECTURES}")
    set(ASM_SOURCE_FILES
        src/main/asm/asmlib.h
        src/main/asm/instrset_x64.asm
        src/main/asm/strstr_see42_x64.asm
        src/main/asm/strstr_see42_v0_x64.asm
        src/main/asm/strstr_see42_v2_x64.asm
    )
elseif ("x86" STREQUAL "${CMAKE_CPU_ARCHITECTURES}")
    set(ASM_SOURCE_FILES
        src/main/asm/asmlib.h
        src/main/asm/instrset_x86.asm
        src/main/asm/strstr_see42_x86.asm
        src/main/asm/strstr_see42_v0_x86.asm
        src/main/asm/strstr_see42_v2_x86.asm
    )
else()
    message("This platform is not support yasm.")
endif()

list(LENGTH ASM_SOURCE_FILES asm_files_length)
if (${asm_files_length} GREATER 0)
    ## STATIC, SHARED
    add_library(asmlib STATIC ${ASM_SOURCE_FILES})
endif()

add_executable(StringMatch ${SOURCE_FILES})
target_link_libraries(StringMatch ${EXTRA_LIBS} asmlib)
