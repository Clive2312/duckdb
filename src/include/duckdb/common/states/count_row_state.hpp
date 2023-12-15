//===----------------------------------------------------------------------===//
//                         DuckDB
//
// duckdb/analyzer/states/count_row_state.hpp
//
//
//===----------------------------------------------------------------------===//
#pragma once

#include "duckdb/common/state_var.hpp"

namespace duckdb {

class CountRowState: public StateVar {
public:
    CountRowState(int id);
    CountRowState(int id, Value val);
    
    void Collector(DataChunk &data);
    void Combiner(vector<Value> &local_values);
};
};
