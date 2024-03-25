#include "duckdb.hpp"
#include "chrono"
#include "iostream"

using namespace duckdb;

int main() {
	DuckDB db(nullptr);

	Connection con(db);

	auto start = high_resolution_clock::now();
	// Test Query 1 -- Only allow aggregates when row count greater than base limit

	// auto result = con.Query("SELECT Sum(rating) From './ratings.parquet' WHERE rating > 4.8");

	// Test Query 2 -- Group by only allowed when each group has row count greater than base limit

	// auto result = con.Query("Select Avg(price), Count(*) From './trade.parquet' WHERE quantity > 3000 GROUP BY stocksymbol;");
	// auto result = con.Query("Select Avg(price), Count(*) From './trade.parquet' WHERE quantity > 2000 GROUP BY stocksymbol;");
	
	// Test Query 3 -- If there is a join, can't access data past certain time

	// auto result = con.Query("Select * FROM './trade.parquet' JOIN './ratings.parquet' ON trade.stocksymbol = ratings.stocksymbol WHERE time > 1000;");
	
	// Test Query 4 -- Query triggering Test 1 and Test 3 match together

	// auto result = con.Query("Select Sum(rating) FROM './trade.parquet' JOIN './ratings.parquet' ON trade.stocksymbol = ratings.stocksymbol WHERE time > 100;");

	//Test Query 5 -- Query for large database
	try {
		auto result = con.Query("Select Sum(quantity), count(*) FROM './trade.parquet' GROUP BY transaction;");
		// result->Print();
	} catch (...) {
		std::cout<<"Query failed policy check.\n";
	}

	auto stop = high_resolution_clock::now();
	auto duration = duration_cast<std::chrono::milliseconds>(stop - start);

	std::cout << duration.count() << std::endl;
	return 0;
}