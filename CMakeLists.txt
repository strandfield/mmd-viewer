cmake_minimum_required(VERSION 3.9)
project(mmd-viewer)

##################################################################
####### C++20
##################################################################

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

##################################################################
####### Qt
##################################################################

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC TRUE)

find_package(Qt6 COMPONENTS Core Gui Widgets OpenGL OpenGLWidgets REQUIRED)


##################################################################
####### mmd-viewer
##################################################################

file(GLOB_RECURSE SRC_FILES ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp)
file(GLOB_RECURSE HDR_FILES ${CMAKE_CURRENT_SOURCE_DIR}/src/*.h)
file(GLOB_RECURSE QRC_FILES ${CMAKE_CURRENT_SOURCE_DIR}/src/*.qrc)
file(GLOB_RECURSE SHADER_FILES 
  ${CMAKE_CURRENT_SOURCE_DIR}/src/*.vert
  ${CMAKE_CURRENT_SOURCE_DIR}/src/*.geom
  ${CMAKE_CURRENT_SOURCE_DIR}/src/*.frag
)

add_executable(mmd-viewer ${HDR_FILES} ${SRC_FILES} ${QRC_FILES} ${SHADER_FILES})

target_include_directories(mmd-viewer PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/src")
target_link_libraries(mmd-viewer Qt6::Core Qt6::Gui Qt6::Widgets Qt6::OpenGL Qt6::OpenGLWidgets)

if(CMAKE_BUILD_TYPE STREQUAL "Release" OR CMAKE_BUILD_TYPE STREQUAL "RelWithDebInfo")
  set_property(TARGET mmd-viewer PROPERTY WIN32_EXECUTABLE true)
endif()
