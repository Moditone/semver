cmake_minimum_required(VERSION 3.0.0)

# Create the project
project(SemVer CXX)

# Set headers
set(HEADERS version.hpp)

source_group(\\ FILES ${HEADERS})
install (FILES ${HEADERS} DESTINATION include/semver)

# Create the target
add_library(SemVer INTERFACE)