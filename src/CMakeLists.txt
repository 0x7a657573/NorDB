cmake_minimum_required(VERSION 3.0.0)

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
    ${LIBRARY_HEADERS_DIR}/NorDB.h
)
set(LIBRARY_SOURCE_DIR
    .
)
set(LIBRARY_SOURCE
    ${LIBRARY_SOURCE_DIR}/NorDB.c
    ${LIBRARY_SOURCE_DIR}/sem_ll.c
    ${LIBRARY_SOURCE_DIR}/ll/Ram_ll.c
)

project(${PROJECT_NAME})

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