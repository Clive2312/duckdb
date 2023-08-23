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
class Policy {

public:
	// string colName;
	// PolicyType policy_type;
	// ExpressionType expression_type;
	// vector<unique_ptr<Policy>> child_policies;
	Json::Value val;

public:
	Policy(Json::Value &policy);
};

} // namespace duckdb
