cmake_minimum_required(VERSION 3.21)

project(amigaguide-on-linux VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

option(AMIGAGUIDE_BUILD_TESTS "Build parser tests" ON)
option(AMIGAGUIDE_BUILD_APP "Build the Qt 6 viewer" ON)

add_library(amigaguide
    src/amigaguide/document.cpp
    src/amigaguide/parser.cpp
    src/amigaguide/renderer_model.cpp
)

target_include_directories(amigaguide
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_compile_features(amigaguide PUBLIC cxx_std_17)

if(AMIGAGUIDE_BUILD_APP)
    find_package(Qt6 REQUIRED COMPONENTS Core Gui Widgets)

    add_executable(amigaguide-reader
        src/app/main.cpp
        src/app/mainwindow.cpp
        src/app/mainwindow.h
    )

    target_link_libraries(amigaguide-reader
        PRIVATE
            amigaguide
            Qt6::Core
            Qt6::Gui
            Qt6::Widgets
    )
endif()

if(AMIGAGUIDE_BUILD_TESTS)
    enable_testing()
    add_executable(amigaguide-tests
        tests/test_parser.cpp
    )
    target_link_libraries(amigaguide-tests PRIVATE amigaguide)
    add_test(NAME amigaguide-parser COMMAND amigaguide-tests)
endif()
