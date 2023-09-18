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
	vector<unique_ptr<Policy>> policies;

public:
	Analyzer(Json::Value &policies_json, LogicalOperator &plan);

	vector<unique_ptr<Policy>> ConditionMatcher();
	void VisitOperator(LogicalOperator &op);

private:
	void ModifyPlan(LogicalOperator &plan, Policy policy);
	bool everything_referenced = false;
	unordered_map<uint8_t, int> operators;
};

} // namespace duckdb