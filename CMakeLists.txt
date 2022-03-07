cmake_minimum_required (VERSION 3.12)

set (PROJECT 
	Signature)
	
set (HEADERS 
	include/signature_file_creator.h
	include/safety_queue.h
	include/thread_pool.h)
 
set (SOURCES 
	src/main.cpp)
	
set (BOOST_COMPONENTS
	program_options)
	
set (Boost_USE_STATIC_LIBS ON)

project(${PROJECT})

find_package (Boost COMPONENTS ${BOOST_COMPONENTS} REQUIRED)
find_package(Threads REQUIRED)

include_directories(${Boost_INCLUDE_DIRS})

add_executable(${PROJECT} ${HEADERS} ${SOURCES})

target_compile_features (${PROJECT_NAME} PRIVATE cxx_std_17)

target_link_libraries (${PROJECT} ${Boost_LIBRARIES})
target_link_libraries(${PROJECT} Threads::Threads)