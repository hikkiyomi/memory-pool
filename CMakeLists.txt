cmake_minimum_required(VERSION 3.0.0)
project(MemoryPoolAllocator VERSION 0.1.0)

include(CTest)
enable_testing()

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_FLAGS "-Wall -Wextra -Wuninitialized -Wshadow -Wno-unused-result -Werror")

add_subdirectory(include)
add_subdirectory(src)
add_subdirectory(bin)

set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)
