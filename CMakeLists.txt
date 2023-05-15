cmake_minimum_required(VERSION 3.10)
set(CMAKE_CXX_STANDARD 20)
project(mood_thieves)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

if (CMAKE_COMPILER_IS_GNUCXX OR CMKAE_CXX_COMPILER_ID MATCHES "Clang")
    add_compile_options(-Wall -Wextra -Wpedantic -Werror)
endif()

find_package(MPI REQUIRED)
include_directories(
    ${CMAKE_SOURCE_DIR}/include
    ${MPI_INCLUDE_PATH}
)

################

add_library(mood_thieves_utils
    src/utils.cpp
)

target_link_libraries(mood_thieves_utils ${MPI_LIBRARIES})

################

add_executable(mood_thieves
    src/main.cpp
)

target_link_libraries(mood_thieves mood_thieves_utils)

################

install(TARGETS mood_thieves mood_thieves_utils
    RUNTIME DESTINATION bin
    LIBRARY DESTINATION lib
)

add_custom_target(run
    COMMAND mpirun -np 4 mood_thieves
    DEPENDS mood_thieves
    WORKING_DIRECTORY ${CMAKE_PROJECT_DIR}
)

add_custom_target(format
    COMMAND bash -c "find ${CMAKE_SOURCE_DIR}/src ${CMAKE_SOURCE_DIR}/include -iname \"*.cpp\" -o -iname \"*.hpp\" |xargs clang-tidy -format-style=file -p ${CMAKE_BINARY_DIR} -fix"
    COMMAND bash -c "find ${CMAKE_SOURCE_DIR}/src ${CMAKE_SOURCE_DIR}/include -iname \"*.cpp\" -o -iname \"*.hpp\" | xargs clang-format --style=file -i"
    VERBATIM
)
