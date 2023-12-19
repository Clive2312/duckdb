#include "duckdb/common/state_var.hpp"

namespace duckdb {

StateVar::StateVar(int id): id(id){}

StateVar::StateVar(int id, Value val): id(id), val(val) {}

void StateVar::Collector(DataChunk &data) {
    return;
}

Value StateVar::Combiner(vector<Value> &local_values) {
    return Value(0);
}

};