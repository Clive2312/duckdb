#include "duckdb/analyzer/policy_function.hpp"

namespace duckdb {

PolicyFunction::PolicyFunction(){};

bool PolicyFunction::inputChecker(StateStore &store) {
    return true;
}

unique_ptr<LogicalOperator> PolicyFunction::getModifiedPlan(unique_ptr<LogicalOperator> plan) {
    return std::move(plan);
}

bool PolicyFunction::matchCondition(LogicalOperator &plan) {
    return false;
}

};