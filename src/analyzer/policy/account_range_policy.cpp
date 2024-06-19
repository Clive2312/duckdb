#include "duckdb/analyzer/policy/account_range_policy.hpp"
#include "duckdb/common/states/count_row_state.hpp"
#include "duckdb/common/states/avg_state.hpp"
#include "duckdb/common/states/median_state.hpp"
#include <iostream>
#include "duckdb/planner/operator/logical_aggregate.hpp"

/*
* CHANGE NOTES 2: A working example of policy_function.cpp
* Policy: Count, average and median Value of a particular column shouldn't exceed specified limits
*/
using namespace std::placeholders;
namespace duckdb {
AccountRangePolicy::AccountRangePolicy(): PolicyFunction(){
    std::cout<<"Constructing Account Range Policy\n";
}

unique_ptr<LogicalOperator> AccountRangePolicy::modifyPlan(unique_ptr<LogicalOperator> op) {
    if(op->type == LogicalOperatorType::LOGICAL_AGGREGATE_AND_GROUP_BY) {
        // TODO: Abstract the logical operator 
        LogicalAggregate &temp = op->Cast<LogicalAggregate>(); // TODO: abstract the type of the operator based on duckdb implementation
        op->inputCheckers.emplace_back(std::bind(&AccountRangePolicy::inputChecker, *this, _1));

        CountRowState *obj = new CountRowState(1, Value(0));
        op->combiners[1] = std::bind(&CountRowState::Combiner, obj, _1);
        op->collectors.emplace_back(std::bind(&CountRowState::Collector, obj, _1));

        AvgState *obj2 = new AvgState(2);
        op->combiners[2] = std::bind(&AvgState::Combiner, obj2, _1);
        op->collectors.emplace_back(std::bind(&AvgState::Collector, obj2, _1));

        MedianState *obj3 = new MedianState(3);
        op->combiners[3] = std::bind(&MedianState::Combiner, obj3, _1);
        op->collectors.emplace_back(std::bind(&MedianState::Collector, obj3, _1));
    }

    for(auto &child: op->children) {
        child = modifyPlan(std::move(child));
    }

    return op;
}

bool AccountRangePolicy::inputChecker(StateStore &store) {
    if(store.GetStateValue(1).front() < 50000) {
        throw std::domain_error("Range policy violation.\n");
        return false;
    }
    if(store.GetStateValue(2).front() < 3000) {
        throw std::domain_error("Average Quantity violation.\n");
        return false;
    }
    if(store.GetStateValue(3).front() < 2000) {
        throw std::domain_error("Average Quantity violation.\n");
        return false;
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