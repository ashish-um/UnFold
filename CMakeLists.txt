cmake_minimum_required(VERSION 3.20)
project(Unfold VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

find_package(Qt6 REQUIRED COMPONENTS Widgets Svg Concurrent)

set(SOURCES
    src/main.cpp
    src/app/MainWindow.cpp
    src/app/MiniMap.cpp
    src/canvas/SpatialView.cpp
    src/canvas/SpatialScene.cpp
    src/items/NodeItem.cpp
    src/items/EdgeItem.cpp
    src/filesystem/FilesystemWorker.cpp
    src/filesystem/FileWatcher.cpp
    src/layout/LayoutEngine.cpp
    src/persistence/WorkspaceManager.cpp
)

set(HEADERS
    src/app/MainWindow.h
    src/app/MiniMap.h
    src/canvas/SpatialView.h
    src/canvas/SpatialScene.h
    src/items/NodeItem.h
    src/items/EdgeItem.h
    src/filesystem/FilesystemWorker.h
    src/filesystem/FileWatcher.h
    src/layout/LayoutEngine.h
    src/persistence/WorkspaceManager.h
)

set(RESOURCES
    resources/resources.qrc
)

add_executable(${PROJECT_NAME} WIN32
    ${SOURCES}
    ${HEADERS}
    ${RESOURCES}
)

target_include_directories(${PROJECT_NAME} PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

target_link_libraries(${PROJECT_NAME} PRIVATE
    Qt6::Widgets
    Qt6::Svg
    Qt6::Concurrent
)
