#include "duckdb/analyzer/policy.hpp"
#include "duckdb/analyzer/analyzer.hpp"
#include "duckdb/planner/operator/logical_comparison_join.hpp"

namespace duckdb {
Analyzer::Analyzer(ClientContext &p_context, Json::Value &policies_json, unique_ptr<LogicalOperator> &plan): context(p_context){
    for(auto &policy_json: policies_json) {
        policies.emplace_back(Policy(policies_json));
    }
    VisitOperator(*plan);
}

vector<Policy> Analyzer::ConditionMatcher() {
    vector<Policy> res;
    for(auto &policy: policies){
        bool matched = true;
        for(auto &cond: policy.conditions){
            if(operators.find(cond.op) == operators.end()){
                matched = false;
                break;
            }
        }
        if(matched){
                res.emplace_back(policy);
        }
    }
    return res;
}
void StandardVisitOperator(LogicalOperator &op) {
	// LogicalOperatorVisitor::VisitOperatorExpressions(op);
	// if (op.type == LogicalOperatorType::LOGICAL_DELIM_JOIN) {
	// 	// visit the duplicate eliminated columns on the LHS, if any
	// 	auto &delim_join = op.Cast<LogicalDelimJoin>();
	// 	for (auto &expr : delim_join.duplicate_eliminated_columns) {
	// 		VisitExpression(&expr);
	// 	}
	// }
	// LogicalOperatorVisitor::VisitOperatorChildren(op);
}
void Analyzer::VisitOperator(LogicalOperator &op) {
	switch (op.type) {
	case LogicalOperatorType::LOGICAL_AGGREGATE_AND_GROUP_BY: {
		// FIXME: groups that are not referenced can be removed from projection
		// recurse into the children of the aggregate
        if(operators.find(op.type) == operators.end()) operators[op.type] = 0;
        operators[op.type]++;
		VisitOperator(*op.children[0]);
		return;
	}
	case LogicalOperatorType::LOGICAL_ASOF_JOIN:
	case LogicalOperatorType::LOGICAL_DELIM_JOIN:
	case LogicalOperatorType::LOGICAL_COMPARISON_JOIN: {
		if (everything_referenced) {
			break;
		}
		auto &comp_join = op.Cast<LogicalComparisonJoin>();
		if (comp_join.join_type == JoinType::MARK || comp_join.join_type == JoinType::SEMI ||
		    comp_join.join_type == JoinType::ANTI) {
			break;
		}
		// FIXME for now, we only push into the projection map for equality (hash) joins
		// FIXME: add projection to LHS as well
		bool has_equality = false;
		for (auto &cond : comp_join.conditions) {
			if (cond.comparison == ExpressionType::COMPARE_EQUAL) {
				has_equality = true;
			}
		}
		if (!has_equality) {
			break;
		}
		// // now, for each of the columns of the RHS, check which columns need to be projected
		// column_binding_set_t unused_bindings;
		// ExtractUnusedColumnBindings(op.children[1]->GetColumnBindings(), unused_bindings);

		// // now recurse into the filter and its children
		// StandardVisitOperator(op);

		// // then generate the projection map
		// GenerateProjectionMap(op.children[1]->GetColumnBindings(), unused_bindings, comp_join.right_projection_map);
		return;
	}
	case LogicalOperatorType::LOGICAL_UNION:
	case LogicalOperatorType::LOGICAL_EXCEPT:
	case LogicalOperatorType::LOGICAL_INTERSECT:
		// for set operations we don't remove anything, just recursively visit the children
		// FIXME: for UNION we can remove unreferenced columns as long as everything_referenced is false (i.e. we
		// encounter a UNION node that is not preceded by a DISTINCT)
		for (auto &child : op.children) {
			VisitOperator(*child);
		}
		return;
	case LogicalOperatorType::LOGICAL_PROJECTION: {
		// then recurse into the children of this projection
        VisitOperator(*op.children[0]);
		return;
	}
	case LogicalOperatorType::LOGICAL_DISTINCT: {
		// distinct, all projected columns are used for the DISTINCT computation
		// mark all columns as used and continue to the children
		// FIXME: DISTINCT with expression list does not implicitly reference everything
		everything_referenced = true;
		break;
	}
	case LogicalOperatorType::LOGICAL_FILTER: {
		auto &filter = op.Cast<LogicalFilter>();
		if (everything_referenced) {
			break;
		}
		// // filter, figure out which columns are not needed after the filter
		// column_binding_set_t unused_bindings;
		// ExtractUnusedColumnBindings(op.children[0]->GetColumnBindings(), unused_bindings);

		// // now recurse into the filter and its children
		// StandardVisitOperator(op);

		// // then generate the projection map
		// GenerateProjectionMap(op.children[0]->GetColumnBindings(), unused_bindings, filter.projection_map);
		return;
	}
	default:
		break;
	}
}

}