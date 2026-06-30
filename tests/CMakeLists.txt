### TEMPLATE_BLOCK: Test code
target_sources(test_tyghbn
  PRIVATE
    test_main.cpp
    test_or_else.cpp
    test_add_one.cpp
)

target_link_libraries(test_tyghbn
  PRIVATE
    tyghbn
)
### TEMPLATE_BLOCK_END: Test code