#include "duckdb/common/types/state_store.hpp"
/*
* CHANGE NOTES 2: State store is used to store and manage those states throughout the query execution.
* a local state is initialized for each data chunk in a query execution in data_chunk.cpp class.
*/
namespace duckdb{

StateStore::StateStore() {};

vector<Value> StateStore::GetStateValue(int id) {
    vector<Value> values = {};
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
            store[entry.first] = vector<Value>(entry.second.begin(), entry.second.end());
        } else {
            for(auto val: entry.second) {
                store[entry.first].emplace_back(val);
            }
        }
    }
    other.store = {};
    return;
}

void StateStore::Move(StateStore &other){
    this->MergeStore(other);
    return;
}

}