<p>The work flow of Query execution in DuckDB:
Step 1: Create a database instance (DuckDB db(nullptr);)
Step 2: Create a connection instance (Connection con(db);)
Step 3: Execute a Query using con.Query("Your SQL query here;")

Elaborating on step 3, connection.cpp calls context->Prepare(query). Prepare function in client_context.cpp then does the following:
1. Parsing string query to ParsedStatement (ParseStatementsInternal(*lock, query);)
2. Calling PrepareInternal which calls CreatePreparedStatement function. (There are other routes based on different args and initialization of db but all lead to CreatePreparedStatement function)
3. CreatePreparedStatement then calls all of Logical planner, optimizer as well as physical planner sequentially

Our pseudo code will be inserted in the CreatePreparedStatement function after logical planner and before optimizer. </p>

```
	auto plan = std::move(planner.plan);
	// extract the result column names from the plan
	result->properties = planner.properties;
	result->names = planner.names;
	result->types = planner.types;
	result->value_map = std::move(planner.value_map);
	result->catalog_version = MetaTransaction::Get(*this).catalog_version;

	if (!planner.properties.bound_all_parameters) {
		return result;
	}

    /* Our modification here */

    if (config.enable_analyzer) {
		profiler.StartPhase("analyzer");
		Analyzer analyzer(result, plan);
        auto valid_policies = analyzer.GetValidPolicies(policies);
		profiler.EndPhase();
	}
```
<p> The analyzer has to take into account all the tables, columns, rows as well as type of operators on those into account for declaring a policy valid. Sample psuedo code: </p>

```
Analyzer::Analyzer(Result &result, Tree &plan) {
	tables, cols, operators = travese(tree); //any tree traversal
											//Operators types will represent joins or aggregate etc.
}

Analyzer::GetValidPolicies(vector<Policy> &policies){
	for each policy in policies{
		if(policy.condition.tables in tables
		 && policy.condition.cols in cols
		 && policy.condition.operators in operators) valid_policies.add(policy)
	}
}
```
<p>
The physical plan will then take vaild_policies as args in the same function. These are the policies that needs to be checked for the given query. Next, the way to overwrite a physical operator workflow is already present in Duckdb (Scan is overwritten for parquet already). Since we don't want to modify the actual working of the operator, we can create a generic wrapper on each physical operator which checks for policy violation after the duckdb's operator has run.
</p>
<p> All that remains is redefining the policy structure in json so as to express all policies in json format correctly. One such sample json could be:</p>

```
{
    “policy”: [{
        “condition”: {
            “operator”: [{
				"type": "aggregate",
				"attribute": ["table1.col1"]
			}],
            “tables”: [“table1", “table2”],
        },
        “action”: {
			// Similar to condition
        },
        “isStatic”: true
    }]
}
```

<p>This can keep on evolving with time as we include more complex policies. Immediate thing that I can see is to add and/or distinction for multiple condition in a single policy.</p>

## Policy enforcement framework
### Idea
<p> We have access to the query logical plan. A function will take this logical plan as input and an array of policy functions to apply onto the logical plan and output a modified logical plan based on the policy to enforce.</p>

``` 
logical_plan PolicyModifiedPlan (logical_plan op, Policy[] polFunctions) {
	foreach func in polFunctions {
		op = polFunctions.apply(op);
	}
	return op;
}
```
<p>
What are these policy functions? These are essentially the functions representing the policy conditions and policy actions combined. They also contain the algorithm on how to do the condition matching as well as policy enforcement in the logical plan tree.
</p>

Let's see an example:

1. Our policy is <b><u>Condition:</u></b> Join on ORDERS, LINEITEM, CUSTOMER; <b><u>Action:</u></b> Can access rows with shipdate within 7 days.

```
LastestOrderPolicy extends PolicyFunction {
	logical_plan apply(logical_plan op) {
		if op.type == JOIN {
			if op.r_child == SCAN and op.l_child == JOIN {
				if tables from r_child, l_child == [ORDERS, CUSTOMER, LINEITEM] 
				then return applyPolicy(op);
			}
			if op.l_child == SCAN and op.r_child == JOIN {
				if tables from r_child, l_child == [ORDERS, CUSTOMER, LINEITEM] 
				then return applyPolicy(op);
			}
		} else return apply(op.child);
	}

	logical_plan applyPolicy(logical_plan op) {
		if(op.type == JOIN) {
			op.outputPolicy.append(outputChecker);
			op.inputPolicy.append(inputChecker);
		}
		return op;
	}

	static const outputChecker(DataChunk result) {
		foreach data in result
			if(data.shipdate < current_time - 7) return false;
		return true;
	}

	static const inputChecker(DataChunk inputChunk) {
		foreach data in result
			if(data.shipdate < current_time - 7) return false;
		return true;
	}
}
```

In this example, the logic of apply is acting as the condition matching on the logical plan tree. Policy operator can choose to do the matching based on different criteria in this function.

The function applyPolicy function is actually modifying the logical_plan nodes to accomodate the policy enforcement logic. Essentially, we are adding the enforcement logic functions into the policy list that the query execution engine will use to check for policy violation.

The functions outputChecker and inputChecker actually takes in the input and output chunk of the data from the operators they are added to. Then based on the return value, we can have a policy violation or not. 

Note that we need to fix the signature of these checker functions which the operator has to overwrite for each PolicyFunction class. 


