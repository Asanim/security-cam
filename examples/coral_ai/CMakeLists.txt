cmake_minimum_required(VERSION 3.16)

project(Classify)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O2 -Wall -Wno-error=comment")
set(CMAKE_POSITION_INDEPENDENT_CODE ON) # For static libraries like TensorFlow Lite

# Source files
set(SOURCES
    minimal.cc
    lstpu.cc
    classify.cc
)

# Include directories
set(INCLUDE_DIRS
    ${CMAKE_BINARY_DIR}/external/install/include
    ${CMAKE_BINARY_DIR}/external/install/usr/local/include
    ${CMAKE_BINARY_DIR}/external/tensorflow-lite/src/
)

# Link directories
set(LINK_DIRS
    ${CMAKE_BINARY_DIR}/external/install/lib
    ${CMAKE_BINARY_DIR}/external/install/usr/local/lib
)

# Libraries
set(LINK_LIBS
    tensorflow-lite
    ${CMAKE_BINARY_DIR}/external/install/lib/libedgetpu.so.1.0
    flatbuffers
    usb-1.0.so
    pthread
    m
    dl

    # tensorflow libraries:
    farmhash
    ruy_allocator
    ruy_context_get_ctx
    ruy_kernel_arm
    ruy_blocking_counter
    ruy_profiler_profiler
    ruy_prepare_packed_matrices
    eight_bit_int_gemm
    cpuinfo
    ruy_profiler_instrumentation
    ruy_apply_multiplier
    ruy_have_built_path_for_avx2_fma
    ruy_system_aligned_alloc
    ruy_kernel_avx2_fma
    fft2d_fftsg2d
    ruy_wait
    ruy_prepacked_cache
    ruy_have_built_path_for_avx512
    ruy_pack_avx2_fma
    ruy_denormal
    ruy_pack_avx
    fft2d_fftsg
    pthreadpool
    flatbuffers
    ruy_trmul
    ruy_kernel_avx
    ruy_pack_arm
    ruy_have_built_path_for_avx
    ruy_context
    ruy_kernel_avx512
    ruy_thread_pool
    ruy_ctx
    ruy_pack_avx512
    ruy_cpuinfo
    ruy_block_map
    ruy_frontend
    ruy_tune
)

# Loop over source files and create executables
foreach(SRC ${SOURCES})
    # Extract the name without extension for the executable
    get_filename_component(EXEC_NAME ${SRC} NAME_WE)

    # Add executable
    add_executable(${EXEC_NAME} ${SRC})

    # Include directories
    target_include_directories(${EXEC_NAME} PRIVATE ${INCLUDE_DIRS})

    # Link directories
    target_link_directories(${EXEC_NAME} PUBLIC ${LINK_DIRS})

    # Link libraries
    target_link_libraries(${EXEC_NAME} ${LINK_LIBS})

    # Ensure dependencies build first
    add_dependencies(${EXEC_NAME} libusb flatc)
endforeach()
