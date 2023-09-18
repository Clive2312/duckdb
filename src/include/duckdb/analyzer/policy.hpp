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
#include "duckdb/common/enums/expression_type.hpp"
#include "duckdb/common/enums/cond_policy_mode.hpp"
namespace duckdb {

struct StatementAST {
	LogicalOperatorType logical_op = LogicalOperatorType::LOGICAL_INVALID;
	ExpressionType op;
	Value attribute;

	StatementAST* l_child = nullptr;
	StatementAST* r_child = nullptr;
	StatementAST(Json::Value &sql);
};

struct Statement {
	CondPolicyMode mode;
	StatementAST* expr = nullptr;
	Statement(Json::Value &statement);
};

class Policy {

public:
	vector<unique_ptr<Statement>> actions;
	vector<unique_ptr<Statement>> conditions;

public:
	Policy(Json::Value &jsonPolicy);
};

} // namespace duckdb
