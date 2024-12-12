
message("Application Version " ${VERSION_STRING})

set(LIBRARIES_TO_LINK
)

## Security Camera executable
add_executable(can_example can_listener.cc)

add_dependencies(can_example can-utils)

target_include_directories(can_example PRIVATE
    ${CMAKE_BINARY_DIR}/external/install/include
)

target_link_directories(can_example PRIVATE
    ${CMAKE_BINARY_DIR}/external/install/lib/
)

target_link_libraries(can_example
    ${LIBRARIES_TO_LINK}
)
