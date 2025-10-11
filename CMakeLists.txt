cmake_minimum_required(VERSION 3.22.1)
project(my_muduo CXX)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_compile_definitions(MYDEBUG)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

add_subdirectory(Base)
add_subdirectory(Net)

#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Werror")

# 測試Reactor板块的代碼
# add_executable(reactor_test test/reactor.cc)
# target_link_libraries(reactor_test PRIVATE Net Base)

# 測試Net板块的代碼
add_executable(Net_test test/Net.cc)
target_link_libraries(Net_test PRIVATE Net Base)