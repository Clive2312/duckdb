//===----------------------------------------------------------------------===//
//                         DuckDB
//
// duckdb/analyzer/policy.hpp
//
//
//===----------------------------------------------------------------------===//

#pragma once
#include "duckdb/common/json/json.h"
#include "duckdb/common/enums/logical_operator_type.hpp"
#include "duckdb/common/types/vector.hpp"
#include "duckdb/common/enums/cond_policy_mode.hpp"
namespace duckdb {

struct StatementAST {
	StatementAST(Json::Value &sql);
	LogicalOperatorType logical_op;
	ExpressionType op;
	Value attribute;
	vector<unique_ptr<StatementAST>> l_children;
	vector<unique_ptr<StatementAST>> r_children;
};
struct Statement {
	CondPolicyMode mode;
	unique_ptr<StatementAST> children;
	Statement(Json::Value &statement);
};

class Policy {

public:
	ExpressionType conjunction;
	vector<unique_ptr<Statement>> policies;
	vector<unique_ptr<Statement>> conditions;

public:
	Policy(Json::Value &jsonPolicy);
};

} // namespace duckdb
