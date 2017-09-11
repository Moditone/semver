cmake_minimum_required(VERSION 3.0.0)

# Create the project
project(SemVer CXX)

# Set headers
set(HEADERS version.hpp)

source_group(\\ FILES ${HEADERS})

if (UNIX)
	install (FILES ${HEADERS} DESTINATION include/moditone/semver)
endif (UNIX)

if(WIN32)
	install (FILES ${HEADERS} DESTINATION moditone/semver)
endif(WIN32)

# Create the target
add_library(SemVer INTERFACE)