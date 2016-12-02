cmake_minimum_required(VERSION 2.6)
project(musicbotd)

#set(CMAKE_BUILD_TYPE Debug)

if(CMAKE_VERSION VERSION_LESS "3.1")
	if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
		set(CMAKE_CXX_FLAGS "-std=c++14 -Wall ${CMAKE_CXX_FLAGS}")
	endif()
else()
	set(CMAKE_CXX_STANDARD 14)
endif()


set(CMAKE_MODULE_PATH cmake)
set(CMAKE_SYSTEM_LIBRARY_PATH ${CMAKE_SYSTEM_LIBRARY_PATH} /usr/lib/x86_64-linux-gnu/)
find_package(CURL REQUIRED)
find_package(Boost 1.62.0 COMPONENTS log)


#set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/lib)
file(GLOB_RECURSE source_files src/*.cpp)
add_executable(${PROJECT_NAME} ${source_files})
target_link_libraries(${PROJECT_NAME}
	${CURL_LIBRARIES}
	pthread
	mpv
	-static-libstdc++
)

# génération des exécutables pour les programmes de test
file(GLOB test_sources test/src/*.cpp)
#set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/test)
foreach(test_source ${test_sources})
    get_filename_component(test_exec ${test_source} NAME_WE)
    add_executable(${test_exec} ${test_source})
    target_link_libraries(${test_exec} ${Boost_LIBRARIES})
    target_include_directories(${test_exec} PRIVATE src)
endforeach(test_source ${test_sources})

# Pour QtCreator (affichage des headers dans l'arborescence)
file(GLOB_RECURSE headers_lib *.h *.hpp *.hxx cmake/*)
add_custom_target(headers SOURCES ${headers_lib})
