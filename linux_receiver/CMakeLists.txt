cmake_minimum_required(VERSION 3.5)
project(LinuxReceiver)

set(CMAKE_CXX_STANDARD 14)
#set(CMAKE_BUILD_TYPE Debug)

find_package(glfw3 REQUIRED)
find_package(OpenGL REQUIRED)
find_package(Boost 1.50 COMPONENTS system REQUIRED )

find_package(PkgConfig REQUIRED)

include_directories ("${PROJECT_SOURCE_DIR}/lib/imgui")
add_subdirectory (lib)

set(SOURCE_FILES main.cpp imgui_impl_glfw.cpp)
add_executable(LinuxReceiver ${SOURCE_FILES})
target_link_libraries (LinuxReceiver ImGui ImGuiDemo glfw GL)
target_link_libraries (LinuxReceiver mysqlcppconn)
target_link_libraries (LinuxReceiver pthread ${Boost_LIBRARIES})

set(CMAKE_VERBOSE_MAKEFILE ON)
set(CMAKE_CXX_FLAGS "-O3")
