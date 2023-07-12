cmake_minimum_required(VERSION 3.5.0)

set(PROJECT_NAME
    CH341
)
set(LIBRARY_NAME
    CH341
)

set(LIBRARY_HEADERS
    "."
    /usr/include/libusb-1.0
)


set(LIBRARY_SOURCE
    ch341.c
    stdafx.c
)


add_library(${LIBRARY_NAME}
    ${LIBRARY_SOURCE}
)

target_include_directories(${LIBRARY_NAME} PUBLIC ".")


TARGET_LINK_LIBRARIES(${PROJECT_NAME}
        usb-1.0
)