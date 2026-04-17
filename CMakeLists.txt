cmake_minimum_required(VERSION 3.14)
project(ccsds-orbit-validator C)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)

# ------------------------------------------------------------------------------
# Static library: ccsds_orbit_validator
# Consumers #include "packet.h" and "validator.h" and link against this target.
# ------------------------------------------------------------------------------
add_library(ccsds_orbit_validator STATIC
    packet.c
    validator.c
)

target_include_directories(ccsds_orbit_validator
    PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}
)

# Link math library (required for sqrt on most POSIX systems)
target_link_libraries(ccsds_orbit_validator PUBLIC m)

# Strict warning flags (GCC / Clang)
if(CMAKE_C_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(ccsds_orbit_validator PRIVATE
        -Wall -Wextra -Wpedantic -Werror
    )
endif()

# MSVC equivalents
if(MSVC)
    target_compile_options(ccsds_orbit_validator PRIVATE /W4 /WX)
endif()

# ------------------------------------------------------------------------------
# Standalone executable: orbit_validator_demo
# ------------------------------------------------------------------------------
add_executable(orbit_validator_demo main.c)

target_link_libraries(orbit_validator_demo PRIVATE ccsds_orbit_validator)

if(CMAKE_C_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(orbit_validator_demo PRIVATE
        -Wall -Wextra -Wpedantic -Werror
    )
endif()

if(MSVC)
    target_compile_options(orbit_validator_demo PRIVATE /W4 /WX)
endif()
