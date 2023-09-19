COPY parts FROM './db1/parts.parquet' (FORMAT 'parquet');
COPY supplier FROM './db1/supplier.parquet' (FORMAT 'parquet');
COPY customer FROM './db1/customer.parquet' (FORMAT 'parquet');
