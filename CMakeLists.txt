cmake_minimum_required(VERSION 3.16)
project(CMakeSFMLProject LANGUAGES CXX)

set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(SFML_STATIC_LIBRARIES ON)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Debug)
endif()

set(SFML_ROOT "/usr/local/lib/cmake/sfml")

find_package(SFML 3 REQUIRED COMPONENTS
    Graphics
    Window
    System
)

add_executable(sand)

target_sources(sand
    PRIVATE
        src/main.cpp
        src/sand/particle_simulation.cpp
        src/multi-threading/thread_pool.cpp
        src/fps/fps.cpp
)

target_include_directories(sand PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

target_link_libraries(sand PRIVATE
    SFML::Graphics
    SFML::Window
    SFML::System
)

target_compile_options(sand PRIVATE -fsanitize=address,undefined -g)
target_link_options(sand PRIVATE -fsanitize=address,undefined)