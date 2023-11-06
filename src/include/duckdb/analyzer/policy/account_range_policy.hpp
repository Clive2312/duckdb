//===----------------------------------------------------------------------===//
//                         DuckDB
//
// duckdb/analyzer/policy/account_range_policy.hpp
//
//
//===----------------------------------------------------------------------===//

#include "duckdb/analyzer/policy_function.hpp"

namespace duckdb {

class AccountRangePolicy: public PolicyFunction {
public:
    AccountRangePolicy();
};

}