//===----------------------------------------------------------------------===//
//                         DuckDB
//
// duckdb/analyzer/policy.hpp
//
//
//===----------------------------------------------------------------------===//

#pragma once
#include "duckdb/common/json/json.h"

namespace duckdb {

enum class PolicyType : uint8_t {
	INVALID = 0,
	FILTER = 1,
	OTHER = 2
};

struct Condition {
	ExpressionClass type;
	vector<string> attributes;
	Condition(Json::Value &condition);
};

class Policy {

public:
	// string colName;
	// PolicyType policy_type;
	// ExpressionType expression_type;
	// vector<unique_ptr<Policy>> child_policies;
	Json::Value policy;
	vector<Condition> conditions;

public:
	Policy(Json::Value &jsonPolicy);
};

} // namespace duckdb
