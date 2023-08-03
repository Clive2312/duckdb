#include "duckdb.hpp"
#include "iostream"
using namespace duckdb;

int main() {
	DuckDB db(nullptr);

	Connection con(db);

	// Create Database with entries
	// con.Query("CREATE TABLE customer(Cust_key INTEGER, Name VARCHAR, Account INTEGER, Region VARCHAR");
	// con.Query("INSERT INTO customer VALUES (1, 'abc', 221, 'a'), (2, 'bcd', 312,'a'), (3, 'def', 234, 'b'), (4, 'ab', 4234, 'a'), (5, 'bc', 234, 'b'), (6, 'de', 23423, 'b')");
	// con.Query("EXPORT DATABASE './db' (FORMAT PARQUET);");
	// con.Query("EXPORT DATABASE './db/csv'");

	// auto res = con.Query("SELECT * FROM duckdb_extensions()");
	// res->Print();

	// auto resu = con.Query("INSTALL 'json';");
	// db.LoadExtension<>();
	// auto res = con.Query("SELECT * FROM duckdb_extensions()");
	// res->Print();

	// con.Query("IMPORT DATABASE './db'");
	// con.Query("INSERT INTO customer VALUES (10, 'abc', NULL, 'a')");
	
	// auto result = con.Query("SELECT * FROM customer");

	auto result = con.Query("SELECT * FROM parquet_scan('./db/customer.parquet', policy_file='./policy/rule.json') WHERE Region='a' ");
	// auto result = con.Query("SELECT * FROM parquet_scan('./db/customer.parquet', policy_file='./policy/rule.json')");

	result->Print();

	// std::cout<<"Print Enter to continue";
	// while(getchar()!= '\n'){};
	
	return 0;
}