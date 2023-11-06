#include "duckdb/analyzer/policy/account_range_policy.hpp"

namespace duckdb {
AccountRangePolicy::AccountRangePolicy(): PolicyFunction(){}

unique_ptr<LogicalOperator> modifyPlan(unique_ptr<LogicalOperator> op) {
    if(op->type == LogicalOperatorType::LOGICAL_AGGREGATE_AND_GROUP_BY) {
        op->inputCheckers.emplace_back(PolicyFunction::inputChecker);
    }

    for(auto &child: op->children) {
        child = modifyPlan(std::move(child));
    }

    return op;
}

bool inputChecker(DataChunk &input) {
    
    for(int i = 0; i < input.size(); i++) {
        auto val = input.GetValue(0, i);
        if(val <= Value(100)) {
            return false;
        }
    }

    return true;
}

unique_ptr<LogicalOperator> PolicyFunction::getModifiedPlan(unique_ptr<LogicalOperator> plan) {
    if(matchCondition(*plan)) {
        return modifyPlan(std::move(plan));
    }
    return std::move(plan);
}



}