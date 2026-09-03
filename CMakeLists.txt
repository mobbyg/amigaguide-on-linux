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
option(AMIGAGUIDE_BUILD_EDITOR "Build the Qt 6 editor" ON)

add_library(amigaguide
    src/amigaguide/document.cpp
    src/amigaguide/destination.cpp
    src/amigaguide/destination_resolver.cpp
    src/amigaguide/navigation.cpp
    src/amigaguide/parser.cpp
    src/amigaguide/renderer_model.cpp
    src/amigaguide/search.cpp
)

target_include_directories(amigaguide
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_compile_features(amigaguide PUBLIC cxx_std_17)

if(AMIGAGUIDE_BUILD_APP OR AMIGAGUIDE_BUILD_EDITOR)
    find_package(Qt6 REQUIRED COMPONENTS Core Gui Widgets)
endif()

if(AMIGAGUIDE_BUILD_APP)
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

if(AMIGAGUIDE_BUILD_EDITOR)
    add_executable(amigaguide-editor
        src/editor/main.cpp
        src/editor/mainwindow.cpp
        src/editor/mainwindow.h
    )

    target_link_libraries(amigaguide-editor
        PRIVATE
            amigaguide
            Qt6::Core
            Qt6::Gui
            Qt6::Widgets
    )
endif()

if(AMIGAGUIDE_BUILD_TESTS)
    enable_testing()

    add_executable(amigaguide-parser-tests
        tests/test_parser.cpp
    )
    target_link_libraries(amigaguide-parser-tests PRIVATE amigaguide)
    add_test(NAME amigaguide-parser COMMAND amigaguide-parser-tests)

    add_executable(amigaguide-destination-tests
        tests/test_destination.cpp
    )
    target_link_libraries(amigaguide-destination-tests PRIVATE amigaguide)
    add_test(NAME amigaguide-destination COMMAND amigaguide-destination-tests)
endif()
