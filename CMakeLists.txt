cmake_minimum_required(VERSION 3.0.0)

set(PROJECT_NAME NorDB_Module)
project(${PROJECT_NAME} VERSION 0.1.0 LANGUAGES C CXX)


set(SOURCE
    src/main.c
)

set(INC
    inc/
)

set(LIB_LIST

)

set(ENABLE_NORDB ON CACHE BOOL "Enable NorDB")
if(ENABLE_NORDB)
    message(STATUS "Enable NorDB")
    add_subdirectory(system)
    list(APPEND LIB_LIST "NorDB_Lib")
endif(ENABLE_NORDB)

add_executable(${PROJECT_NAME} ${SOURCE})

target_link_libraries(${PROJECT_NAME} ${LIB_LIST})

set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)
