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

project(${PROJECT_NAME} VERSION 0.1.0 LANGUAGES C)

add_library(${PROJECT_NAME} src)

add_subdirectory(src)
enable_testing()
add_subdirectory(Demo)

add_test(NAME Ram_ex COMMAND Ram_ex)
set_tests_properties(Ram_ex PROPERTIES
    WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/Demo/Ram"
    TIMEOUT 300
)

add_test(NAME Ram_mt_ex COMMAND Ram_mt_ex)
set_tests_properties(Ram_mt_ex PROPERTIES
    WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/Demo/Ram"
    TIMEOUT 300
)

add_test(NAME FlashMock_ex COMMAND FlashMock_ex)
set_tests_properties(FlashMock_ex PROPERTIES
    WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/Demo/NorFlash/Mock"
    TIMEOUT 300
)

add_test(NAME File_ex COMMAND File_ex)
set_tests_properties(File_ex PROPERTIES
    WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/Demo/File"
    TIMEOUT 300
)

target_link_libraries(${PROJECT_NAME} ${LIBRARIES})

set_target_properties(${PROJECT_NAME} PROPERTIES LINKER_LANGUAGE C) 
