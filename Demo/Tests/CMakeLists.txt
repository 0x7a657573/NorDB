cmake_minimum_required(VERSION 3.22)

set(PROJECT_NAME
    NorTest_ex
)

set(LIBRARY_NAME
    CH341_Lib
)

project(${PROJECT_NAME} C)


set(SRC_FILES
    src/TestConfig.c
    src/TestRoutines.c
    ../../src/NorDB.c
    ../../src/sem_ll.c
    ../../src/ll/SerialNorFlash_ll.c
    ../NorFlash/CH341/ch341.c
    ../NorFlash/CH341/stdafx.c
)

set(INC_FILES
    src/inc/
    ../NorFlash/CH341/
)

add_library(${PROJECT_NAME} STATIC
    ${SRC_FILES}
)

target_include_directories(${PROJECT_NAME} PUBLIC
    "inc"
    "../../src/include"
    "../NorFlash/CH341/"
)
