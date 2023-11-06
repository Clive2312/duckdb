//===----------------------------------------------------------------------===//
//                         DuckDB
//
// duckdb/analyzer/policy_function.hpp
//
//
//===----------------------------------------------------------------------===//
#include "duckdb/planner/logical_operator.hpp"
#include "duckdb/common/types/data_chunk.hpp"

namespace duckdb{

class PolicyFunction {

public:
    unique_ptr<LogicalOperator> getModifiedPlan(unique_ptr<LogicalOperator> plan);
    bool inputChecker(DataChunk &input);
    PolicyFunction();

private:
    bool matchCondition(LogicalOperator &plan);
    unique_ptr<LogicalOperator> modifyPlan(unique_ptr<LogicalOperator> op);
    // TODO: missing tree operations
};

};