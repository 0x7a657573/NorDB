cmake_minimum_required(VERSION 3.5.0)

set(LIBRARY_NAME
    NorDB_Lib
)

set(SOURCE

)
set(LIBRARIES_DIR
    src
)
set(LIBRARIES 
    NorDB
    Platform
    CH341
)

add_library(${LIBRARY_NAME} STATIC Platform NorDB CH341)
set_target_properties(${LIBRARY_NAME}  PROPERTIES LINKER_LANGUAGE C)

add_subdirectory(Platform)
add_subdirectory(NorDB)
add_subdirectory(CH341)

target_include_directories(${LIBRARY_NAME}
    PUBLIC
        Platform
        NorDB
        CH341
)

target_link_libraries(${LIBRARY_NAME} PUBLIC Platform NorDB CH341)

