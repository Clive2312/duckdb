#include "duckdb/analyzer/policy.hpp"
#include "duckdb/common/enums/expression_type.hpp"
#include "duckdb/common/enums/logical_operator_type.hpp"

#include "duckdb/common/json/json.h"

namespace duckdb {

    StatementAST::StatementAST(Json::Value &sql) {
        if(!sql["sql_op"].empty()) {
            auto op = sql["sql_op"].asString();
            if(op.compare("join") == 0) {
                this->logical_op = LogicalOperatorType::LOGICAL_JOIN;
            // } else if(op.compare("join") == 0) {
            //     this->op = LogicalOperatorType::LOGICAL_JOIN;
            // } else if(op.compare("and") == 0) {
            //     this->op = LogicalOperatorType::;
            // } else if(op.compare("join") == 0) {
            //     this->op = LogicalOperatorType::LOGICAL_JOIN;
            // } else if(op.compare("join") == 0) {
            //     this->op = LogicalOperatorType::LOGICAL_JOIN;
            }
        }
    }

    Statement::Statement(Json::Value &statement){
        auto mode = statement["mode"].asString();
        if(mode.compare("sql_match") == 0) {
            this->mode = CondPolicyMode::SQL_MATCH;
        } else if (mode.compare("subquery_match") == 0) {
            this->mode = CondPolicyMode::SUBQUERY_MATCH;
        } else if (mode.compare("bool_exp") == 0) {
            this->mode = CondPolicyMode::BOOL_EXP;
        } else if (mode.compare("filter") == 0) {
            this->mode = CondPolicyMode::FILTER;
        } else {
            this->mode = CondPolicyMode::OTHER;
        }
        
        children = make_uniq<StatementAST>(statement["expr"]);

    }

    Policy::Policy(Json::Value &jsonPolicy) {
        if(jsonPolicy["policies_rel"].asString() == "or") {
            conjunction = ExpressionType::CONJUNCTION_OR;
        } else {
            conjunction = ExpressionType::CONJUNCTION_AND;
        }

        for(auto &cond: jsonPolicy["conditions"]) {
            conditions.emplace_back(make_uniq<Statement>(cond));
        }

        for(auto &policy: jsonPolicy["policies"]) {
            policies.emplace_back(make_uniq<Statement>(policy));
        }
    }
}