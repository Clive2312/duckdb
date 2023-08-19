//===----------------------------------------------------------------------===//
//                         DuckDB
//
// duckdb/policy/policy.hpp
//
//
//===----------------------------------------------------------------------===//

#pragma once

#include "duckdb/planner/logical_operator.hpp"
#include "duckdb/planner/logical_operator_visitor.hpp"
#include "duckdb/analyzer/policy.hpp"

#include <functional>

namespace duckdb {

class Analyzer {
public:
	Analyzer(unique_ptr<LogicalOperator> plan, ClientContext &context, vector<Policy> policies);

	unique_ptr<LogicalOperator> ConditionMatcher(unique_ptr<LogicalOperator> plan);

	ClientContext &context;

private:
	void ModifyPlan(unique_ptr<LogicalOperator> plan, Policy policy);

private:
	unique_ptr<LogicalOperator> plan;
};

} // namespace duckdb