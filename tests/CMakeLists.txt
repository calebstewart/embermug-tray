find_package(Qt6 REQUIRED COMPONENTS Test)

# Test for types utilities
add_executable(test_types test_types.cpp)
target_link_libraries(test_types PRIVATE Ember::ember Qt6::Test)
add_test(NAME test_types COMMAND test_types)

# Test for protocol encoding/decoding
add_executable(test_protocol test_protocol.cpp)
target_link_libraries(test_protocol PRIVATE Ember::ember Qt6::Test)
target_include_directories(test_protocol PRIVATE ${CMAKE_SOURCE_DIR}/lib/src)
add_test(NAME test_protocol COMMAND test_protocol)

# Test for Mug static methods
add_executable(test_mug test_mug.cpp)
target_link_libraries(test_mug PRIVATE Ember::ember Qt6::Test)
add_test(NAME test_mug COMMAND test_mug)
