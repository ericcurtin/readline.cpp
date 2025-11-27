cmake_minimum_required(VERSION 3.14)
project(readline_cpp VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Library sources
set(SOURCES
    src/buffer.cpp
    src/history.cpp
    src/terminal.cpp
    src/readline.cpp
)

# Create library
add_library(readline_cpp ${SOURCES})

# Include directories
target_include_directories(readline_cpp
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
)

# Link libraries
find_package(Threads REQUIRED)
target_link_libraries(readline_cpp PRIVATE Threads::Threads)

# Compiler warnings
if(MSVC)
    target_compile_options(readline_cpp PRIVATE /W4)
else()
    target_compile_options(readline_cpp PRIVATE -Wall -Wextra -Wpedantic)
endif()

# Example executable
add_executable(simple_example examples/simple.cpp)
target_link_libraries(simple_example PRIVATE readline_cpp)

# Installation rules
include(GNUInstallDirs)

install(TARGETS readline_cpp
    EXPORT readline_cpp-targets
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

install(DIRECTORY include/readline
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)

install(EXPORT readline_cpp-targets
    FILE readline_cpp-targets.cmake
    NAMESPACE readline::
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/readline_cpp
)

# Package configuration
include(CMakePackageConfigHelpers)
write_basic_package_version_file(
    "${CMAKE_CURRENT_BINARY_DIR}/readline_cpp-config-version.cmake"
    VERSION ${PROJECT_VERSION}
    COMPATIBILITY AnyNewerVersion
)

install(FILES
    "${CMAKE_CURRENT_BINARY_DIR}/readline_cpp-config-version.cmake"
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/readline_cpp
)
