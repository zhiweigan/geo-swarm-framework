cmake_minimum_required(VERSION 3.10)
project(GEOSWARM)
set(CMAKE_CXX_STANDARD 17)

# Set the executable name and specify the source files
set(SOURCES src/configuration.cpp src/map.cpp)

# Add the third-party directory as an include directory
include_directories(third-party/parlay/include)
include_directories(src)

# Set the default value for the user-defined variable for the specific project in the apps directory
set(APP "sample" CACHE STRING "Name of project in the apps directory")

# Add the apps directory as an include directory
include_directories(apps/${APP})

# Set the policy for the PROJECT_NAME variable
# cmake_policy(SET CMP0077 NEW)

# Add the project source files to the list of sources
list(APPEND SOURCES apps/${APP}/config.cpp apps/${APP}/simulate.cpp)

# Add the executable and specify the sources
add_executable(${APP} ${SOURCES})

set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)
target_link_libraries(${APP} PRIVATE Threads::Threads)