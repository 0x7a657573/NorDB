cmake_minimum_required(VERSION 3.5.0)

set(PROJECT_NAME
    NorDB
)
set(LIBRARY_NAME
    NorDB
)
set(LIBRARY_HEADERS_DIR
    include
)
set(LIBRARY_HEADERS
    include/NorDB.h
)
set(LIBRARY_SOURCE_DIR
    .
)
set(LIBRARY_SOURCE
    ${LIBRARY_SOURCE_DIR}/NorDB.c
    ${LIBRARY_SOURCE_DIR}/sem_ll.c
    ${LIBRARY_SOURCE_DIR}/ll/Ram_ll.c
    ${LIBRARY_SOURCE_DIR}/ll/File_ll.c
    ${LIBRARY_SOURCE_DIR}/ll/SerialNorFlash_ll.c
)

project(${PROJECT_NAME} C)

find_package(Threads REQUIRED)

add_library(${LIBRARY_NAME} STATIC
    ${LIBRARY_HEADERS}
    ${LIBRARY_SOURCE}
)

target_include_directories(${LIBRARY_NAME} PRIVATE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include/>
    $<INSTALL_INTERFACE:include/>
)

target_include_directories(${LIBRARY_NAME} PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

TARGET_LINK_LIBRARIES(${PROJECT_NAME}
        pthread
)