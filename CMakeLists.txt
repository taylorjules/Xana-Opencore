cmake_minimum_required(VERSION 3.14)

project(Xana LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Core)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Core)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Quick Sql)
find_package(Qt${QT_VERSION_MAJOR}Sql REQUIRED)
add_executable(Xana
  main.cpp
  xana.h xana.cpp
  md5.h md5.cpp

)
target_link_libraries(Xana Qt${QT_VERSION_MAJOR}::Core)
target_link_libraries(Xana  Qt${QT_VERSION_MAJOR}::Quick Qt${QT_VERSION_MAJOR}::Sql)
include(GNUInstallDirs)
install(TARGETS Xana
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
