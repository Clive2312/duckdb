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
#include "duckdb/analyzer/policy/account_range_policy.hpp"
#include "duckdb/common/pugixml.hpp"
#include "duckdb/parser/sql_statement.hpp"

#include <functional>

namespace duckdb {

class Analyzer {
public:
	vector<shared_ptr<PolicyFunction>> policyFunctions;
	string policies;

public:
	Analyzer();
	Analyzer(SQLStatement &query);

	string getPolicies();

private:
	string policy_file = "./policy.json";
	pugi::xml_document queryDOM;
    vector<string> createAllCombinations(vector<string> &keys, unordered_map<string, pugi::xpath_node_set> &placeholder_map, Json::Value &policy_sql);
	void generatePolicyInstances(Json::Value policy);
};

} // namespace duckdb