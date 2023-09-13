//===----------------------------------------------------------------------===//
//                         DuckDB
//
// duckdb/common/enums/cond_policy_mode.hpp
//
//
//===----------------------------------------------------------------------===//

#pragma once

#include "duckdb/common/constants.hpp"

namespace duckdb {

//===--------------------------------------------------------------------===//
// Condition and Policy modes and modes
//===--------------------------------------------------------------------===//
enum class CondPolicyMode : uint8_t {
    SQL_MATCH = 1,
    SUBQUERY_MATCH = 2,

    BOOL_EXP = 10,
    FILTER = 11,

    OTHER = 12
};
}