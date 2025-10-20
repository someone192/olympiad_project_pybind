cmake_minimum_required(VERSION 3.13)
project(MyAnnealModule LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)  # 🔥 Для VSCode

find_package(Python REQUIRED COMPONENTS Interpreter Development)
message(STATUS "Python include dirs: ${Python_INCLUDE_DIRS}")  # Debug

# Pybind11
find_package(pybind11 REQUIRED)
message(STATUS "Pybind11 include dirs: ${pybind11_INCLUDE_DIRS}")

add_library(my_module SHARED pybind_module.cpp anneal.cpp)

# 🔥 КРИТИЧНО: Передаём Python includes
target_include_directories(my_module PRIVATE
    ${Python_INCLUDE_DIRS}
    ${pybind11_INCLUDE_DIRS}
)

target_link_libraries(my_module PRIVATE pybind11::module pybind11::embed ${Python_LIBRARIES})

set_target_properties(my_module PROPERTIES
    LIBRARY_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    PREFIX ""
)