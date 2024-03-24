#include "duckdb.hpp"

using namespace duckdb;

int main() {
	DuckDB db(nullptr);

	Connection con(db);
	// con.Query("CREATE TABLE rate (stocksymbol VARCHAR, rating DOUBLE);");
    // con.Query("COPY (SELECT * FROM read_csv('ratings.csv', AUTO_DETECT=true)) TO 'ratings.parquet' (FORMAT PARQUET);");

	con.Query("CREATE TABLE rate (stocksymbol VARCHAR, time INTEGER, quantity INTEGER, price INTEGER, transaction VARCHAR);");
    con.Query("COPY (SELECT * FROM read_csv('trade.csv', AUTO_DETECT=true)) TO 'trade.parquet' (FORMAT PARQUET);");

	return 0;
}