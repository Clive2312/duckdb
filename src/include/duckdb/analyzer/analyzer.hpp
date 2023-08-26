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
#include "duckdb/common/types/vector.hpp"
#include "duckdb/common/unordered_map.hpp"

#include <iostream>
#include <functional>

namespace duckdb {

class Analyzer {
public:
	Analyzer(ClientContext &context, Json::Value &policies_json, unique_ptr<LogicalOperator> &plan);
	vector<Policy> policies;

	vector<Policy> ConditionMatcher();
	void VisitOperator(LogicalOperator &op);
	ClientContext &context;

private:
	void ModifyPlan(unique_ptr<LogicalOperator> plan, Policy policy);
	bool everything_referenced;
	unordered_map<uint8_t, int> operators;
};

} // namespace duckdb