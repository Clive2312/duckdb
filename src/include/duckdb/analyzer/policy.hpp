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

class Policy {
public:
	Policy(Json::Value) {
	};
	virtual ~Policy() {
	};

};

} // namespace duckdb
