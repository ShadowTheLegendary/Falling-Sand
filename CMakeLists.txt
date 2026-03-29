set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# all source files
set(SRC 
    src/main.cpp
    src/sand/particle_simulation.cpp
)

# define the island target
sfml_add_example(falling_sand GUI_APP
                 SOURCES ${SRC}
                 DEPENDS SFML::Graphics SFML::Window SFML::System)

# external dependency headers
target_include_directories(falling_sand SYSTEM PRIVATE 
    ${PROJECT_SOURCE_DIR}/examples/falling_sand
    ${PROJECT_SOURCE_DIR}/examples/falling_sand/src
)