//===----------------------------------------------------------------------===//
//                         DuckDB
//
// duckdb/common/types/state_store.hpp
//
//
//===----------------------------------------------------------------------===//

#pragma once 

#include "duckdb/common/unordered_map.hpp"
#include "duckdb/common/types/value.hpp"

namespace duckdb {

class StateStore {
    public:
    StateStore();
    Value GetStateValue(int id);
    void SetStateValue(int id, Value &val);
    void MergeStore(StateStore &other);
    void Move(StateStore &other);
    public:
    unordered_map<int, vector<Value>> store;

};
}