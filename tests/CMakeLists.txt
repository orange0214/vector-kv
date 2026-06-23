include(FetchContent)

# ---------------------------------------------------------------------------
# GoogleTest
# ---------------------------------------------------------------------------
FetchContent_Declare(
    googletest
    GIT_REPOSITORY https://github.com/google/googletest.git
    GIT_TAG        v1.15.2
    GIT_SHALLOW    TRUE
)

# Prevent GoogleTest from overriding our compiler/linker options on Windows.
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

include(GoogleTest)

# ---------------------------------------------------------------------------
# Test target
#
# Add new *_test.cpp files to this list as modules are implemented.
# ---------------------------------------------------------------------------
add_executable(vectorkv_tests
    smoke_test.cpp
    distance_test.cpp
)

target_link_libraries(vectorkv_tests
    PRIVATE
        VectorKV::core
        GTest::gtest_main
)

gtest_discover_tests(vectorkv_tests)
