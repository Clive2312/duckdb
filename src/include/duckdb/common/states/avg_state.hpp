//===----------------------------------------------------------------------===//
//                         DuckDB
//
// duckdb/common/states/avg_state.hpp
//
//
//===----------------------------------------------------------------------===//
#pragma once

#include "duckdb/common/state_var.hpp"

namespace duckdb {

class AvgState: public StateVar {
public:
    AvgState(int id);
    
    void Collector(DataChunk &data);
    Value Combiner(vector<Value> &local_values);
};
};
