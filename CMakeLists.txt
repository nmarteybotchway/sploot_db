cmake_minimum_required(VERSION 3.16)

project(sploot_db LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(sploot_db main.cpp)

target_include_directories(sploot_db PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/third_party)

include(GNUInstallDirs)
install(TARGETS sploot_db
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)