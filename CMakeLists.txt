#### Setup ####
cmake_minimum_required(VERSION 3.27)
project(Qt-TreeSitterHighlighter)

set(CMAKE_CXX_STANDARD 11)

find_package(Qt6 REQUIRED COMPONENTS Core Gui Widgets)
qt_standard_project_setup()


#### Dependencies ####
add_library(tree-sitter
    ${CMAKE_SOURCE_DIR}/libs/tree-sitter/lib/src/lib.c
)
target_include_directories(tree-sitter PRIVATE
    ${CMAKE_SOURCE_DIR}/libs/tree-sitter/lib/src
    ${CMAKE_SOURCE_DIR}/libs/tree-sitter/lib/include
)

add_library(tree-sitter-cpp
    ${CMAKE_SOURCE_DIR}/libs/tree-sitter-cpp/src/parser.c
    ${CMAKE_SOURCE_DIR}/libs/tree-sitter-cpp/src/scanner.c
    ${CMAKE_SOURCE_DIR}/libs/tree-sitter-cpp/src/tree_sitter/parser.h
)


#### TreeSitterHighlighter ####
add_library(tree-sitter-highlighter
    ${CMAKE_SOURCE_DIR}/src/TreeSitterHighlighter.cpp
    ${CMAKE_SOURCE_DIR}/src/TreeSitterHighlighter.h
)
target_include_directories(tree-sitter-highlighter PRIVATE
    ${CMAKE_SOURCE_DIR}/libs/tree-sitter/lib/include
)
target_link_libraries(tree-sitter-highlighter PRIVATE
    Qt6::Core
    Qt6::Gui
    Qt6::Widgets
    tree-sitter
)


#### Examples ####
qt_add_executable(tree-sitter-highlighter-example
    ${CMAKE_SOURCE_DIR}/examples/main.cpp
)
target_include_directories(tree-sitter-highlighter-example PRIVATE
    ${CMAKE_SOURCE_DIR}/libs/tree-sitter/lib/include
    ${CMAKE_SOURCE_DIR}/src/
)
target_link_libraries(tree-sitter-highlighter-example PRIVATE
    Qt6::Core
    Qt6::Gui
    Qt6::Widgets
    tree-sitter-cpp
    tree-sitter-highlighter
)
