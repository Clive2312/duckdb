#include "duckdb/common/states/count_row_state.hpp"
#include <iostream>
using namespace std::placeholders;
namespace duckdb {
CountRowState::CountRowState(int id): StateVar(id){
}

CountRowState::CountRowState(int id, Value val): StateVar(id, val){
}

void CountRowState::Collector(DataChunk &data) {
    auto val = Value((int)data.size());
    data.store->SetStateValue(id, val);
}

Value CountRowState::Combiner(vector<Value> &local_values) {
    int gval = 0;
    for(auto &local_val: local_values) {
        auto local_val_int = local_val.template GetValue<int>();
        gval += local_val_int;
    }
    return Value((int)gval); 
}
}