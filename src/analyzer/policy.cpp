#include "duckdb/analyzer/policy.hpp"

#include "duckdb/common/json/json.h"

namespace duckdb {
    Policy::Policy(Json::Value &jsonPolicy):val(jsonPolicy){
        
    }
}