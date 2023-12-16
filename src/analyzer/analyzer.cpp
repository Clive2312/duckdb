#include "duckdb/analyzer/policy.hpp"
#include "duckdb/analyzer/analyzer.hpp"
// #include "duckdb/analyzer/policy/account_range_policy.hpp"
// #include "duckdb/planner/operator/logical_comparison_join.hpp"
using namespace std::placeholders;

namespace duckdb {
Analyzer::Analyzer(){
    policyFunctions.emplace_back(make_shared<AccountRangePolicy>());
}

unique_ptr<LogicalOperator> Analyzer::modifiedPlan(unique_ptr<LogicalOperator> op) {
    for(auto &policy: policyFunctions) {
        op = policy->getModifiedPlan(std::move(op));
    }
    return op;
}

unique_ptr<LogicalOperator> separateFunctions(unique_ptr<LogicalOperator> op) {
    for(auto &state:op->states) {
        op->collectors.emplace_back(std::bind(&StateVar::Collector, state, _1));
        op->combiners.emplace_back(std::bind(&StateVar::Combiner, state, _1));
    }
    return op;
}

unique_ptr<LogicalOperator> Analyzer::modifyStateFunctionLocations(unique_ptr<LogicalOperator> op){
    if(op == NULL) return op;

    for(auto &child: op->children) {
        child = std::move(modifyStateFunctionLocations(std::move(child)));
    }
    return separateFunctions(std::move(op));
}   
// Analyzer::Analyzer(Json::Value &policies_json){
//     for(auto &policy: policies_json) {
//         policies.emplace_back(make_shared<Policy>(policy));
//     }
// }

// string GetName(Value &attrib) {
//     string val = attrib.ToString();
//     if(!val.empty()) {
//         if(val[0] != '@') {
//             return StringUtil::Lower(val);
//         }
//         else {
//             return StringUtil::Lower(val.substr(1, val.size() - 1));
//         }
//     }
//     return "";
// }

// bool CompareTables(ConditionAST* cond, LogicalOperator &op){
// 	if(op.type == LogicalOperatorType::LOGICAL_GET) {
// 		return StringUtil::Lower(op.ParamsToString()) == GetName(cond->attribute);
// 	}
// 	return false;
// }

// bool TreeMatcher(ConditionAST* cond, LogicalOperator &op){
//     if(cond == nullptr) return true;
    
//     if(op.type == cond->logical_op || CompareTables(cond, op)) {

//         bool leftMatch = false, rightMatch = false;

//         for(int i = 0; i < op.children.size(); i++) {
//             if(!leftMatch && TreeMatcher(cond->l_child, *op.children[i])){
//                 leftMatch = true;
//             }

//             if(!rightMatch && TreeMatcher(cond->r_child, *op.children[i])){
//                 rightMatch = true;
//             }

//             if(leftMatch && rightMatch) {
//                 return true;
//             }
//         }
//         if(op.children.empty()) return true;
//     } 
//     if(op.children.empty()) return false;

//     bool matched = false;
//     for(int i = 0; i < op.children.size(); i++) {
//         if(!matched && TreeMatcher(cond, *op.children[i])){
//             matched = true;
//         }
//         if(matched) {
//             return true;
//         }
//     }
//     return false;
// }

// bool MatchConditions(vector<shared_ptr<Condition>> &conditions, LogicalOperator &plan){
//     for(auto &cond: conditions) {
//         if(cond->mode == CondPolicyMode::SQL_MATCH && TreeMatcher(cond->expr, plan)) return true;
//     }
//     return false;
// }

// vector<shared_ptr<Policy>> Analyzer::ConditionMatcher(LogicalOperator &plan) {
//     vector<shared_ptr<Policy>> res = {};
//     for(auto &policy: policies){
//         if(MatchConditions(policy->conditions, plan)){
//             res.emplace_back(policy);
//         }
//     }
//     return res;
// }

// void InsertIntoOps(shared_ptr<Action> action, LogicalOperator &op){
//     if(op.type == action->op) {
//         op.actions.emplace_back(action);
//     } 

//     for(auto &child: op.children) {
//         InsertIntoOps(action, *child);
//     }
// }

// unique_ptr<LogicalOperator> Analyzer::MatchAndInsertPolicies(unique_ptr<LogicalOperator> plan) {
//     auto policies = ConditionMatcher(*plan);

//     for(auto policy: policies) {
//         for(auto &action: policy->data_actions) {
//             InsertIntoOps(action, *plan);
//         }
//     }
//     return std::move(plan);
// }


}