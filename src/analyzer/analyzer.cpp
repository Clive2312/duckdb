#include "duckdb/analyzer/policy.hpp"
#include "duckdb/analyzer/analyzer.hpp"

namespace duckdb {
Analyzer::Analyzer(ClientContext &p_context, Json::Value &policies): context(p_context){
}
}