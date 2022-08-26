cmake_minimum_required(VERSION 3.5.0)

set(PROJECT_NAME
    NorDB_ex
)
set(SOURCE_DIR
    Example
)
set(SOURCE
    ${SOURCE_DIR}/main.c
)
set(LIBRARIES_DIR
    src
)
set(LIBRARIES 
    NorDB
)



project(${PROJECT_NAME} C)

add_executable(${PROJECT_NAME} ${SOURCE})

foreach(LIBRARY ${LIBRARIES})
    add_subdirectory("${LIBRARIES_DIR}")
endforeach(LIBRARY)

target_link_libraries(${PROJECT_NAME} ${LIBRARIES})

