add_subdirectory(policy)
add_subdirectory(states)

add_library_unity(
  duckdb_analyzer
  OBJECT
  analyzer.cpp
  policy.cpp
  policy_function.cpp
  state_var.cpp)

set(ALL_OBJECT_FILES
    ${ALL_OBJECT_FILES} $<TARGET_OBJECTS:duckdb_analyzer>
    PARENT_SCOPE)
