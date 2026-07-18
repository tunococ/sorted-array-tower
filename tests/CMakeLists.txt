### TEMPLATE_BLOCK: Test code
target_sources(test_sorted_array_tower
  PRIVATE
    test_main.cpp
    test_skip_array.cpp
    test_bounded_array.cpp
)

target_link_libraries(test_sorted_array_tower
  PRIVATE
    sorted_array_tower
)
### TEMPLATE_BLOCK_END: Test code