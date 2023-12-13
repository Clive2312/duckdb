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
		op = polFunctions.matchCondition(op);
	}
	return op;
}
```
<p>
What are these policy functions? These are essentially the functions representing the policy conditions and policy actions combined. They also contain the algorithm on how to do the condition matching as well as policy enforcement in the logical plan tree.
</p>

Let's see an example:

1. Our policy is <b><u>Condition:</u></b> Join on "ORDERS", "LINEITEM", "CUSTOMER"; <b><u>Action:</u></b> Can access rows with shipdate within 7 days.

```
LastestOrderPolicy extends PolicyFunction {
	logical_plan matchCondition(logical_plan op) {
		if op.type == JOIN {
			if op.r_child == SCAN and op.l_child == JOIN {
				if tables from r_child, l_child == ["ORDERS", "CUSTOMER", "LINEITEM"] 
				then return applyPolicy(op);
			}
			if op.l_child == SCAN and op.r_child == JOIN {
				if tables from r_child, l_child == ["ORDERS", "CUSTOMER", "LINEITEM"] 
				then return applyPolicy(op);
			}
		} else return matchCondition(op.child);
	}

	logical_plan applyPolicy(logical_plan op) {
		if(op.type == JOIN) {
			op.outputChecker.append(outputChecker);
			op.inputChecker.append(inputChecker);
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

## Updates after 10/18

### Policy checker functions

Use a addTaint() function to add a column containing the taint information for the row under the given policy.

Use the checker function to check for policy violations based on taints as well as the values of the row data. 

Let us first define the semantics of our policy function:

```
Class PolicyFunction {
	getModifiedPlan() : return the modified logical plan along with the checkers and tainter functions appended to the operators by calling modifyNodes.

	matchCondition(): Checks if the condition for the policy matches the logical tree or not, and/or throws an error is the policy match is just a query structure match.
	
	/*Tree operations*/

	createNode<T>(): create new nodes to be inserted into the logical tree.
	replaceNodes<T>(): Replaces the input node with another node in the logical tree.
	modifyNodes(): Handles all the modification in the logical tree nodes according to policy enforcement rules. 

	/*Policy enforcement functions*/

	inputChecker() & OutputChecker(): Based on rows in the operator, checks the defined constraints on that rows to determine policy violations. Since taint is also stored as additional attributes, checker functions handles taint checking in the same way.
	inputTainter() & OutputTainter(): same as checker functions but instead of returning an error, adds taint as an additional attributes.
}
```

Examples using these functions:
1. Our policy is <b><u>Condition:</u></b> Join on "ORDERS", "LINEITEM", "CUSTOMER"; <b><u>Action:</u></b> Can access rows with shipdate within 7 days.
```
LastestOrderPolicy extends PolicyFunction {
	
	public getModifiedPlan(logical_plan orig_op){ 
		if(matchCondition(orig_op)) {
			logical_plan modified_op = orig_op;
			modifyNodes(modified_op);
			return modified_op; 
		}
		return orig_op;
	}

	Boolean matchCondition(logical_plan op) {
		if(op == NULL) return false;

		if op.type == JOIN {
			if op.r_child == SCAN and op.l_child == JOIN {
				if tables from r_child, l_child == ["ORDERS", "CUSTOMER", "LINEITEM"] 
				then return true;
			}
			if op.l_child == SCAN and op.r_child == JOIN {
				if tables from r_child, l_child == ["ORDERS", "CUSTOMER", "LINEITEM"] 
				then return true;
			}
		}

		foreach child in op.children 
			if(matchCondition(op.child)) return true;

		return false;		
	}

	void modifyNodes(logical_plan op) {
		if(op == NULL) return;

		if(op.type == JOIN) {
			if op.r_child == SCAN and op.l_child == JOIN {
				if tables from r_child, l_child == ["ORDERS", "CUSTOMER", "LINEITEM"] 
				then op.inputChecker.append(inputChecker);
			}
			if op.l_child == SCAN and op.r_child == JOIN {
				if tables from r_child, l_child == ["ORDERS", "CUSTOMER", "LINEITEM"] 
				then op.inputChecker.append(inputChecker);
			}
		}
		
		if(op.type == SCAN and table from child == "LINEITEM") {
			then op.tainter.append(inputTainter);
		}
		
		modifyNodes(op.child);
	}

	static Boolean inputChecker(DataChunk inputChunk) {
		foreach data in result
			if(data.taint_col_1) return false;
		return true;
	}

	static inputTainter(DataChunk data){
		foreach row in data
			if(row.shipdate < current_time - 7) {
				row.addAttribute(taint_col_1, true);
			}
	}
}
```

2. Our policy is <b><u>Condition:</u></b> If we have an aggregate on account_bal of Customer table; <b><u>Action:</u></b> Rows cannot include data from multiple regions.
```
AggregatePolicy extends PolicyFunction {
	
	public getModifiedPlan(logical_plan orig_op){ 
		if(matchCondition(orig_op)) {
			logical_plan modified_op = orig_op;
			modifyNodes(modified_op);
			return modified_op; 
		}
		return orig_op;
	}

	Boolean matchCondition(logical_plan op) {
		if(op == NULL) return false;

		if op.type == Aggregate and op.col == account_bal {
			then return true;
		} 
		 
		foreach child in op.children 
			if(matchCondition(op.child)) return true;

		return false;
	}

	void modifyNodes(logical_plan op = modified_op) {
		if(op == NULL) return;

		if op.type == Aggregate and op.col == account_bal {
			op.inputChecker.append(inputChecker);
		}

		if op.type == Scan and op.table = "CUSTOMER" {
			replaceNodes<Join>(op, new createNode<Scan>("CUSTOMER"), new createNode<Scan>(REGION));
		}
		modifyNodes(op.child);
	}

	static Boolean inputChecker(DataChunk inputChunk) {
		if count(data.region, distinct = true) > 1 return false;

		return true;
	}
}
```

3. Our policy is <b><u>Condition:</u></b> If there is a like operator in the query; <b><u>Action:</u></b> The like operator is only allowed in the subquery
```
LikeOperatorPolicy extends PolicyFunction {
	
	public getModifiedPlan(logical_plan orig_op){ 
		if(matchCondition(orig_op)) {
			logical_plan modified_op = orig_op;
			modifyNodes(modified_op);
			return modified_op; 
		}
		return orig_op;
	}

	Boolean matchCondition(logical_plan op) {
		if(op == NULL) return false;

		if op.type == Like and op.parent == NULL {
			then Throw(PolicyViolationError: "Policy Violated");
		}

		foreach child in op.children 
			if(matchCondition(op.child)) return true;

		return false;
	}
}
```
4. Our policy is <b><u>Condition:</u></b> Aggregate over SUPPLIER.ACC_BAL <b><u>Action:</u></b> The aggregate cannot be less than 100.
```
MinAggregatePolicy extends PolicyFunction {
	
	public getModifiedPlan(logical_plan orig_op){ 
		if(matchCondition(orig_op)) {
			logical_plan modified_op = orig_op;
			modifyNodes(modified_op);
			return modified_op; 
		}
		return orig_op;
	}

	Boolean matchCondition(logical_plan op) {
		if(op == NULL) return false;

		if op.type == AGGREGATE and op.col == SUPPLIET.ACC_BAL 
		then return true; 

		foreach child in op.children 
			if(matchCondition(op.child)) return true;
		
		return false;
	}

	void modifyNodes(logical_plan op = modified_op) {
		if(op == NULL) return;

		if op.type == AGGREGATE and op.col == SUPPLIET.ACC_BAL 
		then op.outputChecker.append(outputChecker);

		modifyNodes(op.child);
	}

	static Boolean outputChecker(DataChunk res) {
		if(res.val < 100) return false;
		return true;
	}

}
```
5. Our policy is <b><u>Condition:</u></b> GROUP BY on EMPLOYEE.ZIPCODE <b><u>Action:</u></b> Each output group should include at least 20 employees.
```
MinGroupbyPolicy extends PolicyFunction {
	
	public getModifiedPlan(logical_plan orig_op){ 
		if(matchCondition(orig_op)) {
			logical_plan modified_op = orig_op;
			modifyNodes(modified_op);
			return modified_op; 
		}
		return orig_op;
	}

	Boolean matchCondition(logical_plan op) {
		if(op == NULL) return false;

		if op.type == GROUP BY and op.col == EMPLOYEE.ZIPCODE
		then return true; 

		foreach child in op.children 
			if(matchCondition(op.child)) return true;
		
		return false;
	}

	void modifyNodes(logical_plan op = modified_op) {
		if(op == NULL) return;

		if op.type == GROUP BY and op.col == EMPLOYEE.ZIPCODE
		then op.outputChecker.append(outputChecker);

		modifyNodes(op.child);
	}

	static Boolean outputChecker(DataChunk res) {
		foreach group in res{
			if(count(group.rows) < 20) return false;
		}
		return true;
	}

}
```
### Stateful vs stateless checker functions

1. Operations checking on the values of rows or taints (such as value of a field out of certain range or presence of a taint) doesn't require states to persist.
2. In case of the policy checkers are calculating values like averages, sums, row counts etc., the policy checkers have to be stateful.

Two possible solutions for handling stateful nature of second types of policy checkers:

1. If the operator that the policy checkers are added on is already pipeline breakers, then all the data will pass through that operator before the query execution moves foward. This means that the operator already have a local state for all parallel threads to store the data, a global state for accumulating the final data and a finalize function which handles the synchronization between those threads. We can just add the values of averages, sums, row counts etc. in the local state by each thread running in parallel, combine it in the finalize step into the global state and use the final values into the single instance of checker functions using these global values.

The problem then reduces to converting all the operators which uses policy checker functions into pipeline breaker.

2. The other approach would be instead of adding the checker function into the existing operators, create a custom operator which is always a pipeline breaker. We can then use the same approach as in Step 1 to manage the parallel execution in that custom operator, or we can use some other synchronization methods according to our needs in the custom operator.

## Updates after 10/25

### Execution model

The current execution model assumption is as follows:

1. Checker functions requires a set of data for policy checking.
2. Checker functions can take certain actions on that data. 

For part 1, checker functions may require a single data or multiple/full data. 

When the checker function needs full data before performing any action on the query, the query will wait at immediate next sink in the logical tree before releasing it's data. If all the checker functions returns true, the sink operator further releases the data, otherwise the checker operator throws an error appropriately. (We can also wait at the last sink operator instead of waiting at multiple sink operators. This will reduce the implementation complexity but might result into worse performance overall). 

Now, we need to maintain states for each checker function. One proposed model to handle states here:

- Each parallel execution of operators will run their own thread local state collection
- Each state collection function will collect local state information
- Propagate the local state information along with the query result data to parent operators
- The immediate sink operator (where the query is waiting for policy validation) will then combine the states from the local informations. 
- The final states will then be used for policy validation checking

Policy operator will define how to collect and combine the states in the policy definitions. One of the possible semantics:
```
Struct StateVar {
	string name; // name of the state variable
	Value value; // The value of the state var. Value can be string, bool, int etc.

	Value Collector(DataChunk, value); // how to collect the value from the datachunk of the query result 
	Value Combiner(StateVar[]); // how to combine the values from multiple local states into one result
}
```
Let us look at one example:

Our policy is <b><u>Condition:</u></b> If we have an aggregate on account_bal of Customer table; <b><u>Action:</u></b> Rows cannot include data from multiple regions.
```
AggregatePolicy extends PolicyFunction {
	vector<state> {s1, s2, s3}; // assume
	public getModifiedPlan(logical_plan orig_op){ 
		if(matchCondition(orig_op)) {
			logical_plan modified_op = orig_op;
			modifyNodes(modified_op);
			return modified_op; 
		}
		return orig_op;
	}

	Boolean matchCondition(logical_plan op) {
		if(op == NULL) return false;

		if op.type == Aggregate and op.col == account_bal {
			then return true;
		} 
		 
		foreach child in op.children 
			if(matchCondition(op.child)) return true;

		return false;
	}

	void modifyNodes(logical_plan op = modified_op) {
		if(op == NULL) return;

		if op.type == Aggregate and op.col == account_bal {
			State count_state = State("count_rows", (int)0, collector, combiner);
			op.states.append(count_state);
			op.inputChecker.append(inputChecker);
		}

		if op.type == Scan and op.table = "CUSTOMER" {
			replaceNodes<Join>(op, new createNode<Scan>("CUSTOMER"), new createNode<Scan>(REGION));
		}
		modifyNodes(op.child);
	}

	int collector(DataChunk result, int value) {
		return value + result.size();
	}
	
	int combiner(int[] values){
		int res = 0;
		foreach val in values:
			res += val;
		return res;
	}

	void initState(State name) {
		
	}

	boolean inputChecker(DataChunk inputChunk, State[] states) {
		int count = states.find("count_rows").value;
		if count > 1 return false;

		return true;
	}
}
```

