cmake_minimum_required(VERSION 3.11)

project(TransportCatalogue CXX)
set(CMAKE_CXX_STANDARD 17)
add_compile_options(-g)

find_package(Protobuf REQUIRED)
find_package(Threads REQUIRED)

protobuf_generate_cpp(PROTO_SRCS PROTO_HDRS transport_catalogue.proto svg.proto 
                      map_renderer.proto graph.proto)

set (TRANSPORT_CATALOGUE_FILES domain.h geo.cpp geo.h graph.h json_builder.cpp json_builder.h
     json_reader.cpp json_reader.h json.cpp json.h main.cpp map_renderer.cpp map_renderer.h
     ranges.h router.h svg.cpp svg.h transport_catalogue.cpp 
     transport_catalogue.h serialization.cpp serialization.h)

add_executable(transport_catalogue ${TRANSPORT_CATALOGUE_FILES} ${PROTO_SRCS} ${PROTO_HDRS})

target_include_directories(transport_catalogue PUBLIC ${Protobuf_INCLUDE_DIRS})
target_include_directories(transport_catalogue PUBLIC ${CMAKE_CURRENT_BINARY_DIR})

target_link_libraries(transport_catalogue ${Protobuf_LIBRARY} Threads::Threads) 

