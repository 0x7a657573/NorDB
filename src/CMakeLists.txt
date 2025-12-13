cmake_minimum_required(VERSION 3.22)

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

option(NORDB_ENABLE_LOG "Enable NorDB core logging macros (info_log/err_log)" OFF)

add_library(${LIBRARY_NAME} STATIC
    ${LIBRARY_HEADERS}
    ${LIBRARY_SOURCE}
)

if(NORDB_ENABLE_LOG)
    target_compile_definitions(${LIBRARY_NAME} PUBLIC NORDB_ENABLE_LOG)
endif()

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