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

struct DataConstraint {
	string table;
	string column;
	int min_count = -1;
	int max_count = -1;
	DataConstraint(Json::Value &data);
};
struct Action {
	LogicalOperatorType op;
	vector<DataConstraint *> dConstraints;
	Action(Json::Value &action);
};
struct ConditionAST {
	LogicalOperatorType logical_op = LogicalOperatorType::LOGICAL_INVALID;
	ExpressionType op;
	Value attribute;

	ConditionAST* l_child = nullptr;
	ConditionAST* r_child = nullptr;
	ConditionAST(Json::Value &sql);
};

struct Condition {
	CondPolicyMode mode;
	ConditionAST* expr = nullptr;
	Condition(Json::Value &statement);
};

class Policy {

public:
	vector<shared_ptr<Condition>> actions;
	vector<shared_ptr<Condition>> conditions;
	vector<shared_ptr<Action>> data_actions;

public:
	Policy(Json::Value &jsonPolicy);
};

} // namespace duckdb
