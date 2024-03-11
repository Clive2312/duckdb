#include "duckdb.hpp"
#include "iostream"
using namespace duckdb;

int main() {
	DuckDB db(nullptr);

	Connection con(db);
	
	// Test Query 1 -- Only allow aggregates when row count greater than base limit

	// auto result = con.Query("SELECT Sum(rating) From './ratings.parquet' WHERE rating > 4.8");

	// Test Query 2 -- Group by only allowed when each group has row count greater than base limit

	// auto result = con.Query("Select Avg(price), Count(*) From './trade.parquet' WHERE quantity > 3000 GROUP BY stocksymbol;");
	// auto result = con.Query("Select Avg(price), Count(*) From './trade.parquet' WHERE quantity > 2000 GROUP BY stocksymbol;");
	
	// Test Query 3 -- If there is a join, can't access data past certain time

	auto result = con.Query("Select * FROM './trade.parquet' JOIN './ratings.parquet' ON trade.stocksymbol = ratings.stocksymbol WHERE time > 1000;");


	// result->Print();
	
	return 0;
}