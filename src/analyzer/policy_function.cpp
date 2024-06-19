#include "duckdb/analyzer/policy_function.hpp"

/*
* CHANGE NOTES 2: For the idea "state collection for policy violation". 
* Essentially, each policy is an instance of a policy function that dictates 
* how to collect(and merge together) certain states from different data chunks 
* matchCondition function specifies when to trigger the policy checking based on the structure of the logical plan.
* Then, using these states, a inputChecker is specified which dictates when a policy violation occurs.
*/
namespace duckdb {

PolicyFunction::PolicyFunction(){};

bool PolicyFunction::inputChecker(StateStore &store) {
    return true;
}

unique_ptr<LogicalOperator> PolicyFunction::getModifiedPlan(unique_ptr<LogicalOperator> plan) {
    return std::move(plan);
}

bool PolicyFunction::matchCondition(LogicalOperator &plan) {
    return false;
}

};