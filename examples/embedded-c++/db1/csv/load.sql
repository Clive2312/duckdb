COPY parts FROM './db1/csv/parts.csv' (FORMAT 'csv', quote '"', delimiter ',', header 0);
COPY supplier FROM './db1/csv/supplier.csv' (FORMAT 'csv', quote '"', delimiter ',', header 0);
COPY customer FROM './db1/csv/customer.csv' (FORMAT 'csv', quote '"', delimiter ',', header 0);
