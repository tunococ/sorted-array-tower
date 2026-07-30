### TEMPLATE_BLOCK: Test code
target_sources(test_sorted_array_tower
  PRIVATE
    test_main.cpp
    test_skip_array.cpp
    test_bounded_array.cpp
    test_bounded_vector.cpp
    test_binary_heap.cpp
    test_merge_iterator.cpp
    test_merge.cpp
)

target_link_libraries(test_sorted_array_tower
  PRIVATE
    sorted_array_tower
)
### TEMPLATE_BLOCK_END: Test code