#include "duckdb/analyzer/policy.hpp"
#include "duckdb/common/enums/expression_type.hpp"
#include "duckdb/common/enums/logical_operator_type.hpp"

#include "duckdb/common/json/json.h"

namespace duckdb {
    Condition::Condition(Json::Value &condition){
        auto op = condition["operator"].asString();

        if(op.compare("aggregate") == 0) {
            this->op = LogicalOperatorType::LOGICAL_AGGREGATE_AND_GROUP_BY;
        }

        // for(auto &attr:condition["attributes"]);
    }
    Policy::Policy(Json::Value &jsonPolicy):policy(jsonPolicy["policy"]){
        for(auto &cond: jsonPolicy["conditions"]){
            conditions.emplace_back(Condition(cond));
        }
    }
}