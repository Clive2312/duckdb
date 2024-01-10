add_library_unity(
  duckdb_analyzer_states
  OBJECT
  count_row_state.cpp
  avg_state.cpp
  median_state.cpp)
set(ALL_OBJECT_FILES
    ${ALL_OBJECT_FILES} $<TARGET_OBJECTS:duckdb_analyzer_states>
    PARENT_SCOPE)
