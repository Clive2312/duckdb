//===----------------------------------------------------------------------===//
//                         DuckDB
//
// duckdb/analyzer/policy/account_range_policy.hpp
//
//
//===----------------------------------------------------------------------===//
#pragma once

#include "duckdb/analyzer/policy_function.hpp"

namespace duckdb {

class AccountRangePolicy: public PolicyFunction {
public:
    unique_ptr<LogicalOperator> getModifiedPlan(unique_ptr<LogicalOperator> plan);
    bool inputChecker(DataChunk &input);
    AccountRangePolicy();

private:
    bool matchCondition(LogicalOperator &plan);
    unique_ptr<LogicalOperator> modifyPlan(unique_ptr<LogicalOperator> op);
    // TODO: missing tree operations
};
};
