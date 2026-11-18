cmake_minimum_required(VERSION 3.22)

set(SOURCE
	FlashMock_example.c
	SpiFlashMock.c
	../../../src/NorDB.c
	../../../src/sem_ll.c
	../../../src/ll/SerialNorFlash_ll.c
	../../Tests/src/TestRoutines.c
	../../Tests/src/TestConfig.c
)

set(INC_FILES
	../../../src/include
	../../Tests/inc
)

project(FlashMock_ex C)

add_executable(FlashMock_ex ${SOURCE})
target_include_directories(FlashMock_ex PRIVATE ${INC_FILES} .)

find_package(Threads REQUIRED)
target_link_libraries(FlashMock_ex pthread)
