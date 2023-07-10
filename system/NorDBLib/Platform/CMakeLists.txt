cmake_minimum_required(VERSION 3.5.0)

set(LIBRARY_NAME
    Platform
)

set(LIBRARY_HEADERS
    "."
)

set(LIBRARY_SOURCE
    File_ll.c
    Ram_ll.c
    SpiFlash_ll.c
)

add_library(${LIBRARY_NAME} 
    ${LIBRARY_HEADERS}
    ${LIBRARY_SOURCE}
)


target_include_directories(${LIBRARY_NAME} PUBLIC ".")
