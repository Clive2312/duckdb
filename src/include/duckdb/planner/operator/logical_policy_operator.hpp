//===----------------------------------------------------------------------===//
//                         DuckDB
//
// duckdb/planner/operator/logical_policy_operator.hpp
//
//
//===----------------------------------------------------------------------===//

#pragma once

#include "duckdb/planner/logical_operator.hpp"

namespace duckdb {

//! LogicalPolicyOperator represents a dummy scan returning a single row
class LogicalPolicyOperator : public LogicalOperator {
public:
	static constexpr const LogicalOperatorType TYPE = LogicalOperatorType::LOGICAL_POLICY_OPERATOR;

public:
	explicit LogicalPolicyOperator(idx_t table_index)
	    : LogicalOperator(LogicalOperatorType::LOGICAL_POLICY_OPERATOR), table_index(table_index) {
	}

	idx_t table_index;

public:
	vector<ColumnBinding> GetColumnBindings() override {
		return {ColumnBinding(table_index, 0)};
	}

	idx_t EstimateCardinality(ClientContext &context) override {
		return 1;
	}
	void Serialize(FieldWriter &writer) const override;
	static unique_ptr<LogicalOperator> Deserialize(LogicalDeserializationState &state, FieldReader &reader);
	vector<idx_t> GetTableIndex() const override;
	string GetName() const override;

protected:
	void ResolveTypes() override {
		if (types.size() == 0) {
			types.emplace_back(LogicalType::INTEGER);
		}
	}
};
} // namespace duckdb
