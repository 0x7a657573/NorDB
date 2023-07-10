cmake_minimum_required(VERSION 3.5.0)

set(LIBRARY_NAME
    NorDB
)

set(LIBRARY_HEADERS
    "."
)


set(LIBRARY_SOURCE
    NorDB.c
    sem_ll.c
)

find_package(Threads REQUIRED)

add_library(${LIBRARY_NAME}
    ${LIBRARY_HEADERS}
    ${LIBRARY_SOURCE}
)

target_include_directories(${LIBRARY_NAME} PUBLIC ".")


TARGET_LINK_LIBRARIES(${LIBRARY_NAME}
        pthread
)