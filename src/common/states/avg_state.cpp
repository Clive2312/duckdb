#include "duckdb/common/states/avg_state.hpp"
#include <iostream>
using namespace std::placeholders;
namespace duckdb {
AvgState::AvgState(int id): StateVar(id){
}

void AvgState::Collector(DataChunk &data) {
    auto val = Value((int)data.size());
    data.store->SetStateValue(id, val);
}

Value AvgState::Combiner(vector<Value> &local_values) {
    int gval = 0;
    for(auto &local_val: local_values) {
        auto local_val_int = local_val.template GetValue<int>();
        gval += local_val_int;
    }
    return Value((int)gval); 
}
}