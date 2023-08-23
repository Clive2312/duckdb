#include "duckdb/analyzer/policy.hpp"
#include "duckdb/common/enums/expression_type.hpp"

#include "duckdb/common/json/json.h"

namespace duckdb {
    Condition::Condition(Json::Value &condition){
        auto op = condition["operator"].asString();
        if(op.compare("aggregate")) type = ExpressionClass::AGGREGATE;
        for(auto &attr:condition["attributes"]);
    }
    Policy::Policy(Json::Value &jsonPolicy):policy(jsonPolicy["policy"]){
        for(auto &cond: jsonPolicy["conditions"]){
            conditions.emplace_back(Condition(cond));
        }
    }
}