#include "duckdb/analyzer/policy.hpp"
#include "duckdb/common/enums/expression_type.hpp"
#include "duckdb/common/enums/logical_operator_type.hpp"
#include "duckdb/common/json/json.h"

/*
* CHANGE NOTES 2: This file repesent a policy object that was used by "putting filter checks on operators"  
* Essentially, each policy is a json object and is used to specify the physical plan operator that needs additional filters for policy violations.
* NOT used anywhere currently
*/
namespace duckdb {

    DataConstraint::DataConstraint(Json::Value &data) {
        table = data["table"].asString();
        column = data["column"].asString();
        min_count = data["min_count"].asInt();
        max_count = data["max_count"].asInt();
    }

    Action::Action(Json::Value &action) {
        auto op = action["op"].asString();
        if(op.compare("aggregare") == 0) {
            this->op = LogicalOperatorType::LOGICAL_AGGREGATE_AND_GROUP_BY;
        }
        for(auto data: action["data"]) {
            dConstraints.emplace_back(new DataConstraint(data));
        }
    }

    ConditionAST::ConditionAST(Json::Value &expr) {
        attribute = Value::CreateValue(expr["literal"].asString());

        if(!expr["sql_op"].empty()) {
            auto op = expr["sql_op"].asString();
            if(op.compare("join") == 0) {
                logical_op = LogicalOperatorType::LOGICAL_COMPARISON_JOIN;
            } else if(op.compare("=") == 0) {
                this->op = ExpressionType::COMPARE_EQUAL;
            } else if(op.compare("aggregate") == 0 || op.compare("count") == 0 || op.compare("average") == 0) {
                logical_op = LogicalOperatorType::LOGICAL_AGGREGATE_AND_GROUP_BY;
            } 
            // else if(op.compare("and") == 0) {
            //     this->op = LogicalOperatorType::;
            // } else if(op.compare("join") == 0) {
            //     this->op = LogicalOperatorType::LOGICAL_JOIN;
            // } else if(op.compare("join") == 0) {
            //     this->op = LogicalOperatorType::LOGICAL_JOIN;
        }
        if(!expr["left"].empty()) {
            l_child = new ConditionAST(expr["left"]);
        } 
        if(!expr["right"].empty()) {
            r_child = new ConditionAST(expr["right"]);
        } 
    }

    Condition::Condition(Json::Value &statement){
        expr = new ConditionAST(statement["expr"]);

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
            actions.emplace_back(make_shared<Condition>(actionJson));
        }

        for(auto &cond: jsonPolicy["conditions"]) {
            conditions.emplace_back(make_shared<Condition>(cond));
        }

        for(auto &action: jsonPolicy["data_actions"]) {
            data_actions.emplace_back(make_shared<Action>(action));
        }

    }
}