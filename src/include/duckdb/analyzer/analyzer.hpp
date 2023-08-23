//===----------------------------------------------------------------------===//
//                         DuckDB
//
// duckdb/policy/analyzer.hpp
//
//
//===----------------------------------------------------------------------===//

#pragma once

#include "duckdb/planner/logical_operator.hpp"
#include "duckdb/planner/logical_operator_visitor.hpp"
#include "duckdb/analyzer/policy.hpp"
#include "duckdb/common/json/json.h"
#include <iostream>
#include <functional>

namespace duckdb {

class Analyzer {
public:
	Analyzer(ClientContext &context, Json::Value &policies);

	unique_ptr<LogicalOperator> ConditionMatcher(unique_ptr<LogicalOperator> plan);

	ClientContext &context;

private:
	void ModifyPlan(unique_ptr<LogicalOperator> plan, Policy policy);
};

} // namespace duckdb