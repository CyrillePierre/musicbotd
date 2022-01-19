cmake_minimum_required(VERSION 2.6)
project(musicbotd)

set(CMAKE_BUILD_TYPE Debug)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
	set(CMAKE_CXX_FLAGS "-std=c++17 -Wall -pedantic -DCHEVAL ${CMAKE_CXX_FLAGS}")
endif()

set(CMAKE_MODULE_PATH cmake)
set(CMAKE_SYSTEM_LIBRARY_PATH ${CMAKE_SYSTEM_LIBRARY_PATH} /usr/lib/x86_64-linux-gnu/)
find_package(CURL REQUIRED)

# lib log
file(GLOB_RECURSE log_sources lib/log/*.cpp)
add_library(log ${log_sources})

# lib net
file(GLOB_RECURSE net_sources lib/net/*.cpp)
add_library(net ${net_sources})

include_directories(lib)

# main program
file(GLOB_RECURSE source_files src/*.cpp)
add_executable(${PROJECT_NAME} ${source_files})
target_include_directories(${PROJECT_NAME} PRIVATE ${Boost_INCLUDE_DIRS})
target_link_libraries(${PROJECT_NAME}
	${CURL_LIBRARIES}
	${Boost_LIBRARIES}
	pthread
	mpv
	log
	net
	-static-libstdc++
)

# génération des exécutables pour les programmes de test
file(GLOB test_sources test/*.cpp)
foreach(test_source ${test_sources})
    get_filename_component(test_exec ${test_source} NAME_WE)
    add_executable(${test_exec} ${test_source})
    target_include_directories(${test_exec} PRIVATE src)
endforeach(test_source ${test_sources})

# Pour QtCreator (affichage des headers dans l'arborescence)
file(GLOB_RECURSE headers_lib *.h *.hpp *.hxx cmake/*)
add_custom_target(headers SOURCES ${headers_lib})

target_link_libraries(test_log log)
