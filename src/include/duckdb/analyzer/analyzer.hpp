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
	vector<shared_ptr<Policy>> policies;

public:
	// Analyzer(Json::Value &policies_json);
	void VisitOperator(LogicalOperator &op);
	unique_ptr<LogicalOperator> MatchAndInsertPolicies(unique_ptr<LogicalOperator> plan);

private:
	void ModifyPlan(LogicalOperator &plan, Policy policy);
	bool everything_referenced = false;
	vector<shared_ptr<Policy>> ConditionMatcher(LogicalOperator &plan);
	unordered_map<uint8_t, int> operators;
};

} // namespace duckdb