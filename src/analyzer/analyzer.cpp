#include "duckdb/analyzer/policy.hpp"
#include "duckdb/analyzer/analyzer.hpp"
// #include "duckdb/planner/operator/logical_comparison_join.hpp"

namespace duckdb {
Analyzer::Analyzer(Json::Value &policies_json){
    for(auto &policy: policies_json) {
        policies.emplace_back(make_uniq<Policy>(policy));
    }
}

vector<unique_ptr<Policy>> Analyzer::ConditionMatcher(LogicalOperator &plan) {
    vector<unique_ptr<Policy>> res = {};
    for(auto &policy: policies){
        if(MatchConditions(policy->conditions, plan)){
            res.emplace_back(std::move(policy));
        }
    }
    return res;
}

bool MatchConditions(vector<unique_ptr<Statement>> &conditions, LogicalOperator &plan){
    for(auto &cond: conditions) {
        if(cond->mode == CondPolicyMode::SQL_MATCH && TreeMatcher(cond->expr, plan)) return true;
    }
    return false;
}

bool TreeMatcher(StatementAST* cond, LogicalOperator &op){
    if(cond == nullptr) return true;
    if(op.type == cond->logical_op || StringUtil::Lower(op.GetName()) == GetName(cond->attribute)) {

        bool leftMatch = false, rightMatch = false;

        for(int i = 0; i < op.children.size(); i++) {
            if(!leftMatch && TreeMatcher(cond->l_child, *op.children[i])){
                leftMatch = true;
            }

            if(!rightMatch && TreeMatcher(cond->l_child, *op.children[i])){
                rightMatch = true;
            }

            if(rightMatch && rightMatch) {
                return true;
            }
        }
        return false;
    } 
    if(op.children.empty()) return false;
    return TreeMatcher(cond, *op.children[0]);
}

string GetName(Value &attrib) {
    string val = attrib.ToString();
    if(!val.empty()) {
        if(val[0] != '@') {
            return StringUtil::Lower(val);
        }
        else {
            return StringUtil::Lower(val.substr(1, val.size() - 1));
        }
    }
    return "";
}

}