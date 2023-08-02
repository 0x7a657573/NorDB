cmake_minimum_required(VERSION 3.22)

set(PROJECT_NAME
    NorDB_ex
)

set(LIBRARIES_DIR
    src
)

set(LIBRARIES 
    NorDB
)

project(${PROJECT_NAME} C)

add_library(${PROJECT_NAME} src)

add_subdirectory(src)
add_subdirectory(Demo)

target_link_libraries(${PROJECT_NAME} ${LIBRARIES})

set_target_properties(${PROJECT_NAME} PROPERTIES LINKER_LANGUAGE C) 
