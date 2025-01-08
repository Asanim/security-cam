add_subdirectory(can)
add_subdirectory(coral_ai)
add_subdirectory(mqtt)
add_subdirectory(rk3399)
add_subdirectory(shadow)

# A dummy target to build all examples in this directory
add_custom_target(examples)
add_dependencies(examples )