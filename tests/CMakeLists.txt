include(FetchContent)
FetchContent_Declare(
        googletest
        # Specify the commit you depend on and update it regularly.
        URL https://github.com/google/googletest/archive/934542165899c786cb5d8a710529c37184730183.zip
)
# For Windows: Prevent overriding the parent project's compiler/linker settings
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

add_executable(bst_test_ treap_test.cpp)

target_include_directories(bst_test_ PRIVATE ${googletest_SOURCE_DIR}/googletest/include/)
target_link_libraries(bst_test_ gtest_main)

add_test(NAME bst_test COMMAND bst_test_)
