
message("Application Version " ${VERSION_STRING})

include_directories(
    ${CMAKE_SOURCE_DIR}/src
)

set(LIBRARIES_TO_LINK
    opencv_core
    opencv_imgproc
    opencv_imgcodecs
    opencv_highgui
    opencv_videoio
)

set(SOURCES
    src/main.cc]
)

## Security Camera executable
add_executable(${PROJECT_NAME} main.cc)

add_dependencies(${PROJECT_NAME} opencv)

target_include_directories(${PROJECT_NAME} PRIVATE
    ${CMAKE_BINARY_DIR}/external/install/include
)

target_link_directories(${PROJECT_NAME} PRIVATE
    ${CMAKE_BINARY_DIR}/external/install/lib/
)

target_link_libraries(${PROJECT_NAME}
    ${LIBRARIES_TO_LINK}
)
