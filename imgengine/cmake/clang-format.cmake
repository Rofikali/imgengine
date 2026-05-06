# cmake/clang-format.cmake
# Integrate clang-format into the CMake build as `format` and `format-check` targets.

find_program(CLANG_FORMAT_EXE
    NAMES
      clang-format-16 clang-format-15 clang-format-14 clang-format-13 clang-format
)

if(NOT CLANG_FORMAT_EXE)
    message(STATUS "[fmt] clang-format not found; format targets disabled")
    return()
endif()

find_program(PYTHON3_EXE NAMES python3)
if(NOT PYTHON3_EXE)
    message(STATUS "[fmt] python3 not found; format-check target disabled")
endif()

# Collect candidate source files to format (recursive)
file(GLOB_RECURSE CLANG_FORMAT_FILES
    "${CMAKE_SOURCE_DIR}/src/*.c"
    "${CMAKE_SOURCE_DIR}/src/*.h"
    "${CMAKE_SOURCE_DIR}/include/*.h"
    "${CMAKE_SOURCE_DIR}/include/*.c"
    "${CMAKE_SOURCE_DIR}/*.c"
    "${CMAKE_SOURCE_DIR}/*.h"
)

# Exclude build output directory if present
if(CMAKE_BINARY_DIR)
    list(FILTER CLANG_FORMAT_FILES EXCLUDE REGEX "${CMAKE_BINARY_DIR}")
endif()

if(NOT CLANG_FORMAT_FILES)
    message(STATUS "[fmt] No source files found for clang-format")
    return()
endif()

add_custom_target(format
    COMMAND ${CLANG_FORMAT_EXE} -style=file -i ${CLANG_FORMAT_FILES}
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    COMMENT "Run clang-format (style from .clang-format)"
)

if(PYTHON3_EXE)
    add_custom_target(format-check
        COMMAND ${PYTHON3_EXE} ${CMAKE_SOURCE_DIR}/scripts/clang-format-check.py --clang ${CLANG_FORMAT_EXE}
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
        COMMENT "Check source files are formatted with clang-format"
    )
endif()
