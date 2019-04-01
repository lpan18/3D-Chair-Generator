cmake_minimum_required(VERSION 3.5)
project(nanogui_test)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14")

add_subdirectory(lib/nanogui)
include_directories(lib/nanogui/include)
include_directories(${NANOGUI_EXTRA_INCS})
add_definitions(${NANOGUI_EXTRA_DEFS})

include_directories(lib/embed-resource)
add_subdirectory(lib/embed-resource)

embed_resources(MyResources shader/vert.glsl shader/frag.glsl)

set(OBJ_SOURCE_FILES
    obj_view.cpp
    Mesh.cpp
    Mesh.h
    ChairMixer.cpp
    ChairMixer.h
    ObjBuffer.cpp
    ObjBuffer.h
    W_edge.cpp
    W_edge.h
    Constants.h)

add_executable(chair ${OBJ_SOURCE_FILES} ${MyResources})
target_link_libraries(chair nanogui ${NANOGUI_EXTRA_LIBS})

