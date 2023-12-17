#include "duckdb/common/types/state_store.hpp"

namespace duckdb{

StateStore::StateStore() {};

Value StateStore::GetStateValue(int id) {
    if(store.find(id) == store.end()) {
        return Value(0);
    }
    return store[id].back(); //fix this for using combiner function
};

void StateStore::SetStateValue(int id, Value &val) {
    if(store.find(id) == store.end()) {
        store[id] = {};
    }

    store[id].push_back(val);
    return;
}


void StateStore::MergeStore(StateStore &other) {
    for(auto &entry: other.store){
        if(store.find(entry.first) == store.end()) {
            store[entry.first] = entry.second;
        }
        else {
            for(auto &val: entry.second) {
                store[entry.first].emplace_back(val);
            }
        }
    } 

    return;
}

void StateStore::Move(StateStore &other){
    this->MergeStore(other);
    return;
}

}