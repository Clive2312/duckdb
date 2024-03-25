#include "duckdb/analyzer/policy.hpp"
#include "duckdb/analyzer/analyzer.hpp"
#include "fstream"
#include "iostream"

#define MATCH "match"
#define STATIC_CHECK "static_check"
#define POLICY_SQL "policy_sql"
#define PLACEHOLDER '$'
#define EMPTY_WHERE "WHERE $"
#define EMPTY_WHERE_SIZE 9

namespace duckdb {

Analyzer::Analyzer(SQLStatement& query){
    queryDOM.load_string((query.ToXMLString()).c_str());
}

void Analyzer::queryTreeStructChecker(Json::Value &policy){
    if(policy[STATIC_CHECK]) {
        auto static_check_xpath = policy[STATIC_CHECK].asCString();
        auto static_check = queryDOM.select_node(static_check_xpath).node();
        if(static_check) {
            throw std::domain_error("Policy check failed!\n");
        }
    }
    return;
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
                queryTreeStructChecker(policy);
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

void Analyzer::generatePolicyInstances(Json::Value &policy) {
    auto it = policy.begin();
    unordered_map<string, pugi::xpath_node_set> placeholder_map;
    vector<string> keys;

    for( ;it != policy.end(); it++) {
        string key = it.key().asString();
        if(isPlaceholder(key))
        {
            keys.emplace_back(key);
            placeholder_map[key] = queryDOM.select_nodes(it->asCString());
        }
    }

    vector<string> completed_policies = createAllCombinations(keys, placeholder_map, policy[POLICY_SQL]);

    for(int i = 0; i < completed_policies.size(); i++) {
        auto policy = completed_policies[i];
        auto empty_where = policy.find(EMPTY_WHERE) ;
        if(empty_where != std::string::npos) {
            policy = policy.replace(empty_where, EMPTY_WHERE_SIZE, "");
        }
        policies += policy;
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
            if(placeholder_map[lastkey].size() == 0) {
                completed_policies.emplace_back(policy);
                continue;
            }
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

}