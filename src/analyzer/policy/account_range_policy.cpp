#include "duckdb/analyzer/policy/account_range_policy.hpp"
#include "duckdb/analyzer/states/count_row_state.hpp"
#include <iostream>
#include "duckdb/planner/operator/logical_aggregate.hpp"

using namespace std::placeholders;
namespace duckdb {
AccountRangePolicy::AccountRangePolicy(): PolicyFunction(){
    std::cout<<"Constructing Account Range Policy\n";
}

unique_ptr<LogicalOperator> AccountRangePolicy::modifyPlan(unique_ptr<LogicalOperator> op) {
    if(op->type == LogicalOperatorType::LOGICAL_AGGREGATE_AND_GROUP_BY) {
        // TODO: Abstract the logical operator 
        LogicalAggregate &temp = op->Cast<LogicalAggregate>(); // TODO: abstract the type of the operator based on duckdb implementation
        // std::cout<<"Inserting range policy "<<temp.group_index<<"##"<<temp.aggregate_index<<"##"<<temp.groups[0]->ToString()<<"\n";
        op->inputCheckers.emplace_back(std::bind(&AccountRangePolicy::inputChecker, *this, _1));
        op->states.emplace_back(make_uniq<CountRowState>(1, Value(0)));
    }

    for(auto &child: op->children) {
        child = modifyPlan(std::move(child));
    }

    return op;
}

bool AccountRangePolicy::inputChecker(DataChunk &data) {
    
    for(int i = 0; i < input.size(); i++) { // TODO: 1. We need to abstract how the operator is working on the table data
        auto val = input.GetValue(0, i); // assume operator knows the catalog and schema
        if(val <= Value(300)) {
            return false;
        }
    }
    
    return true;
}                           

bool AccountRangePolicy::matchCondition(LogicalOperator &op) {
    if(op.type == LogicalOperatorType::LOGICAL_AGGREGATE_AND_GROUP_BY) {
        return true;    
    }
    bool match = false;
    for(auto &child: op.children) {
        match = match || matchCondition(*child);
        if(match) {
            return true;
        }
    }
    return false;
}

unique_ptr<LogicalOperator> AccountRangePolicy::getModifiedPlan(unique_ptr<LogicalOperator> plan) {
    if(matchCondition(*plan)) {
        return modifyPlan(std::move(plan));
    }
    return std::move(plan);
}



}