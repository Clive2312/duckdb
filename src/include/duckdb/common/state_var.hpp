//===----------------------------------------------------------------------===//
//                         DuckDB
//
// duckdb/common/state_var.hpp
//
//
//===----------------------------------------------------------------------===//
#pragma once

#include "duckdb/planner/logical_operator.hpp"
#include "duckdb/common/types/data_chunk.hpp"

namespace duckdb{

class StateVar {

public:
    int id;
    Value val;

    StateVar(int id);
    StateVar(int id, Value val);

    virtual void Collector(DataChunk &data);
    virtual Value Combiner(vector<Value> &local_values);
};

};