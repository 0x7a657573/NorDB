cmake_minimum_required(VERSION 3.22)

set(SOURCE
    SPI_example.c
    ../../src/NorDB.c
    ../../src/sem_ll.c
    ../../src/ll/SerialNorFlash_ll.c
    CH341/ch341.c
    CH341/stdafx.c
    ../Tests/src/TestRoutines.c
    ../Tests/src/TestConfig.c
)

set(INC_FILES
    ../../src/include
    CH341
    ../Tests/inc/
)

set(PROJECT_NAME
    NorFlash_ex
)

set(LIBRARY_NAME
    CH341_Lib
)


project(${PROJECT_NAME} C)

add_executable(${PROJECT_NAME} ${SOURCE} ${INC_FILES})

target_include_directories(${PROJECT_NAME} PUBLIC ${INC_FILES})

find_package(Threads REQUIRED)

TARGET_LINK_LIBRARIES(${PROJECT_NAME}
        pthread
        usb-1.0
)
