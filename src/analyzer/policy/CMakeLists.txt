add_library_unity(
  duckdb_analyzer_policies
  OBJECT
  account_range_policy.cpp)
set(ALL_OBJECT_FILES
    ${ALL_OBJECT_FILES} $<TARGET_OBJECTS:duckdb_analyzer_policies>
    PARENT_SCOPE)
