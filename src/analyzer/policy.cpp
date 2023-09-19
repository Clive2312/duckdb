#include "duckdb/analyzer/policy.hpp"
#include "duckdb/common/enums/expression_type.hpp"
#include "duckdb/common/enums/logical_operator_type.hpp"
#include "duckdb/common/json/json.h"

namespace duckdb {
    StatementAST::StatementAST(Json::Value &expr) {
            
        attribute = Value::CreateValue(expr["literal"].asString());

        if(!expr["sql_op"].empty()) {
            auto op = expr["sql_op"].asString();
            if(op.compare("join") == 0) {
                logical_op = LogicalOperatorType::LOGICAL_COMPARISON_JOIN;
            }

            // } else if(op.compare("join") == 0) {
            //     this->op = LogicalOperatorType::LOGICAL_JOIN;
            // } else if(op.compare("and") == 0) {
            //     this->op = LogicalOperatorType::;
            // } else if(op.compare("join") == 0) {
            //     this->op = LogicalOperatorType::LOGICAL_JOIN;
            // } else if(op.compare("join") == 0) {
            //     this->op = LogicalOperatorType::LOGICAL_JOIN;

            if(op.compare("==") == 0) {
                this->op = ExpressionType::COMPARE_EQUAL;
            }
            
        }
        if(!expr["left"].empty()) {
            l_child = new StatementAST(expr["left"]);
        } 
        if(!expr["right"].empty()) {
            r_child = new StatementAST(expr["right"]);
        } 
    }

    Statement::Statement(Json::Value &statement){
        expr = new StatementAST(statement["expr"]);

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
    }

    Policy::Policy(Json::Value &jsonPolicy) {

        for(auto &actionJson: jsonPolicy["policies"]) {
            actions.emplace_back(make_uniq<Statement>(actionJson));
        }

        for(auto &cond: jsonPolicy["conditions"]) {
            conditions.emplace_back(make_uniq<Statement>(cond));
        }

    }
}