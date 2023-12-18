#include "duckdb/common/types/state_store.hpp"

namespace duckdb{

StateStore::StateStore() {};

vector<Value> StateStore::GetStateValue(int id) {
    if(HasStateId(id)) {
        return store[id]; //fix this for using combiner function
    }
    return {};
}

void StateStore::ReplaceStateValue(int id, Value val) {
    store[id] = {val};
}

void StateStore::SetStateValue(int id, Value &val) {
    if(store.find(id) == store.end()) {
        store[id] = {};
    }

    store[id].push_back(val);
    return;
}

bool StateStore::HasStateId(int id){
    if(store.find(id) == store.end()) {
        return false;
    }
    return true;
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