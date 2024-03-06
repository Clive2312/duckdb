#include "duckdb/analyzer/policy.hpp"
#include "duckdb/analyzer/analyzer.hpp"
#include "fstream"
// #include "duckdb/analyzer/policy/account_range_policy.hpp"
// #include "duckdb/planner/operator/logical_comparison_join.hpp"
// using namespace std::placeholders;

#define MATCH "match"
#define POLICY_SQL "policy_sql"
#define PLACEHOLDER '$'

namespace duckdb {

Analyzer::Analyzer(SQLStatement& query){
    queryDOM.load_string((query.ToXMLString()).c_str());
}

string Analyzer::getPolicies(){
    if(policies.size() == 0) {
        std::ifstream handle(policy_file);
		Json::Reader reader;
		Json::Value completeJson;
		reader.parse(handle, completeJson);

        for(auto &policy: completeJson){
		    auto matched = queryDOM.select_node(policy[MATCH].asCString()).node();
            if(matched) {
                generatePolicyInstances(policy);
            }
	    }
    }
    return policies;
}

bool isPlaceholder(string key){
    if(key.size() > 0 && key[0] == PLACEHOLDER) return true;
    return false;
}

void Analyzer::generatePolicyInstances(Json::Value policy) {
    auto it = policy.begin();
    unordered_map<string, pugi::xpath_node_set> placeholder_map;
    vector<string> keys;

    for( ;it != policy.end(); it++) {
        string key = it.key().asString();
        if(isPlaceholder(key))
        {
            keys.emplace_back(key);
            std::cout<<key<<'\n';
            placeholder_map[key] = queryDOM.select_nodes(it->asCString());
        }
    }

    vector<string> completed_policies = createAllCombinations(keys, placeholder_map, policy[POLICY_SQL]);

    for(auto &p: completed_policies) {
        policies += p;
    }
}

vector<string> Analyzer::createAllCombinations(vector<string> &keys, unordered_map<string, pugi::xpath_node_set> &placeholder_map, Json::Value &policy_sql){
    if(keys.empty()) return { policy_sql.asCString() };

    auto lastkey = keys.back();
    keys.pop_back();

    auto partial_policies = createAllCombinations(keys, placeholder_map, policy_sql);
    vector<string> completed_policies = {};

    for(auto policy: partial_policies) {
        auto lastkey_pos = policy.find(lastkey);
    
        if(lastkey_pos != string::npos) {
            auto it = placeholder_map[lastkey].begin();

            for(; it != placeholder_map[lastkey].end(); it++) {

                std::stringstream ss;
                it->node().nodeToSQL(ss);

                auto completed_policy = policy.replace(lastkey_pos, lastkey.size(), ss.str());
                completed_policies.emplace_back(completed_policy);
            }
        } else {
            completed_policies.emplace_back(policy);
        }
    }

    return completed_policies;
}

// Analyzer::Analyzer(){
//     policyFunctions.emplace_back(make_shared<AccountRangePolicy>());
// }

// unique_ptr<LogicalOperator> Analyzer::modifiedPlan(unique_ptr<LogicalOperator> op) {
//     for(auto &policy: policyFunctions) {
//         op = policy->getModifiedPlan(std::move(op));
//     }
//     return op;
// }

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