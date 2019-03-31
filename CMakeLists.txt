cmake_minimum_required(VERSION 3.5)
project(nanogui_test)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14")

add_subdirectory(lib/nanogui)
include_directories(lib/nanogui/include)
include_directories(${NANOGUI_EXTRA_INCS})
add_definitions(${NANOGUI_EXTRA_DEFS})

find_package( OpenCV REQUIRED )
find_package( OpenGL REQUIRED)
find_package( GLUT REQUIRED)

include_directories(lib/embed-resource)
add_subdirectory(lib/embed-resource)

embed_resources(MyResources shader/vert.glsl shader/frag.glsl)

set(OBJ_SOURCE_FILES
    obj_view.cpp
    Mesh.cpp
    Mesh.h
    ObjBuffer.cpp
    ObjBuffer.h
    W_edge.cpp
    W_edge.h
    Constants.h)

add_executable(chair ${OBJ_SOURCE_FILES} ${MyResources})
target_link_libraries(chair nanogui ${NANOGUI_EXTRA_LIBS} ${OpenCV_LIBS} ${OPENGL_GL_LIBRARY} ${GLUT_LIBRARIES})


