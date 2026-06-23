# ---------------------------------------------------------------------------
# Example executables
#
# Each example is a small standalone program linked against the core library.
# ---------------------------------------------------------------------------
add_executable(basic_demo basic_demo.cpp)
target_link_libraries(basic_demo PRIVATE VectorKV::core)
