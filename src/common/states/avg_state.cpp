#include "duckdb/common/states/avg_state.hpp"
#include "duckdb/common/pair.hpp"
#include <iostream>
using namespace std::placeholders;
namespace duckdb {
AvgState::AvgState(int id): StateVar(id){
}

void AvgState::Collector(DataChunk &data) {
    Value count = Value((int_fast64_t)data.size());
    int_fast64_t sum = 0;
    for (idx_t i = 0; i < data.size(); i++) {
        sum += data.GetValue(1, i).template GetValue<int>();
    }
    child_list_t<Value> child;
    child.emplace_back(make_pair("count", std::move(count)));
    child.emplace_back(make_pair("sum", Value(sum)));
    auto val = Value::STRUCT(std::move(child));
    data.store->SetStateValue(id, val);
}

Value AvgState::Combiner(vector<Value> &local_values) {//[Struct1, struct2...]
    int_fast64_t gcount = 0;
    int_fast64_t gsum = 0;
    for(auto &local_val: local_values) {
        auto &types = StructType::GetChildTypes(local_val.type());
        auto &values = StructValue::GetChildren(local_val);
        for(int i = 0; i < values.size(); i++) {
            if(types[i].first == "count") {
                gcount += values[i].template GetValue<int_fast64_t>();
            }
            if(types[i].first == "sum") {
                gsum += values[i].template GetValue<int_fast64_t>();
            }
        }
    }
    return Value(((double)gsum/(double)gcount)); 
}

}