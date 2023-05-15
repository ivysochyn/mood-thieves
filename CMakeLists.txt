cmake_minimum_required(VERSION 3.10)
set(CMAKE_CXX_STANDARD 20)
project(mood_thieves)

if (CMAKE_COMPILER_IS_GNUCXX OR CMKAE_CXX_COMPILER_ID MATCHES "Clang")
    add_compile_options(-Wall -Wextra -Wpedantic -Werror)
endif()

# Find OpenMPI
find_package(MPI REQUIRED)
include_directories(${MPI_INCLUDE_PATH})

# Add the executable
add_executable(mood_thieves src/main.cpp)

target_link_libraries(mood_thieves ${MPI_LIBRARIES})

add_custom_target(run
    COMMAND mpirun -np 4 mood_thieves
    DEPENDS mood_thieves
    WORKING_DIRECTORY ${CMAKE_PROJECT_DIR}
)

install(TARGETS mood_thieves DESTINATION bin)

add_custom_target(format
    COMMAND bash -c "find ${CMAKE_SOURCE_DIR}/src -iname \"*.cpp\" -o -iname \"*.hpp\" |xargs clang-tidy -format-style=file -p ${CMAKE_BINARY_DIR} -fix"
    COMMAND bash -c "find ${CMAKE_SOURCE_DIR}/src -iname \"*.cpp\" -o -iname \"*.hpp\" | xargs clang-format --style=file -i"
    VERBATIM
)
