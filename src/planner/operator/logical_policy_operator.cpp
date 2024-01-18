#include "duckdb/planner/operator/logical_policy_operator.hpp"

#include "duckdb/common/field_writer.hpp"
#include "duckdb/main/config.hpp"

namespace duckdb {

void LogicalPolicyOperator::Serialize(FieldWriter &writer) const {
	writer.WriteField(table_index);
}

unique_ptr<LogicalOperator> LogicalPolicyOperator::Deserialize(LogicalDeserializationState &state, FieldReader &reader) {
	auto table_index = reader.ReadRequired<idx_t>();
	return make_uniq<LogicalPolicyOperator>(table_index);
}

vector<idx_t> LogicalPolicyOperator::GetTableIndex() const {
	return vector<idx_t> {table_index};
}

string LogicalPolicyOperator::GetName() const {
	return LogicalOperator::GetName();
}

vector<ColumnBinding> LogicalPolicyOperator::GetColumnBindings() {
	auto first_bindings = children[0]->GetColumnBindings();

    for(int i = 1; i < children.size(); i++) {
        auto next_bindings = children[i]->GetColumnBindings();
        first_bindings.insert(first_bindings.begin(), next_bindings.begin(), next_bindings.end());
    }

	return first_bindings;
}

} // namespace duckdb
