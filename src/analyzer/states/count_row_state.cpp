#include "duckdb/analyzer/states/count_row_state.hpp"
#include <iostream>
using namespace std::placeholders;
namespace duckdb {
CountRowState::CountRowState(int id): StateVar(id){
}

CountRowState::CountRowState(int id, Value val): StateVar(id, val){
}

void CountRowState::Collector(DataChunk &data) {
    auto intval = val.template GetValue<int>();
    val = Value(intval + (int)data.size());
}

void CountRowState::Combiner(vector<Value> &local_values) {
    for(auto &local_val: local_values) {
        auto valint = val.template GetValue<int>();
        auto local_val_int = local_val.template GetValue<int>();
        val = Value(valint + local_val_int);
    }
}
}