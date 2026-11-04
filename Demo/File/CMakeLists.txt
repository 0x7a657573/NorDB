cmake_minimum_required(VERSION 3.22)


set(SOURCE
    File_example.c
    ../../src/NorDB.c
    ../../src/sem_ll.c
    ../../src/ll/File_ll.c
    ../Tests/src/TestRoutines.c
    ../Tests/src/TestConfig.c
)

set(INC_FILES
    ../../src/include
    ../Tests/inc/
)

set(PROJECT_NAME
    File_ex
)

project(${PROJECT_NAME} C)

add_executable(${PROJECT_NAME} ${SOURCE})

target_include_directories(${PROJECT_NAME} PRIVATE ${INC_FILES})

# the tests do ~650k writes, too slow with a sync per write on a real disk
target_compile_definitions(${PROJECT_NAME} PRIVATE NORDB_FILE_NO_FSYNC)

find_package(Threads REQUIRED)

TARGET_LINK_LIBRARIES(${PROJECT_NAME}
        pthread
)
