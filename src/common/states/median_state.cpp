#include "duckdb/common/states/median_state.hpp"
#include "duckdb/common/pair.hpp"
#include <iostream>
using namespace std::placeholders;
namespace duckdb {
MedianState::MedianState(int id): StateVar(id){
}

void MedianState::Collector(DataChunk &data) {
    vector<Value> values;
    for (idx_t i = 0; i < data.size(); i++) {
        values.emplace_back(data.GetValue(1, i));
    }
    auto val = Value::LIST(std::move(values));
    data.store->SetStateValue(id, val);
}

Value MedianState::Combiner(vector<Value> &local_values) {//[vector<Values>, vector<Values>...]
    vector<int> gvalues;
    for(auto &local_val: local_values) {

        auto &values = ListValue::GetChildren(local_val);
        for(int i = 0; i < values.size(); i++) {
            gvalues.emplace_back(values[i].template GetValue<int>());
        }
    }

    sort(gvalues.begin(), gvalues.end());
    auto sz = gvalues.size();
    if(sz % 2) {
        return Value(gvalues[sz/2]);
    }
    return Value(((float)gvalues[sz/2] + (float)gvalues[sz/2 - 1])/2); 
}

}