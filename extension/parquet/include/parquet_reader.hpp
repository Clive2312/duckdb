//===----------------------------------------------------------------------===//
//                         DuckDB
//
// parquet_reader.hpp
//
//
//===----------------------------------------------------------------------===//

#pragma once

#include "duckdb.hpp"
#ifndef DUCKDB_AMALGAMATION
#include "duckdb/planner/table_filter.hpp"
#include "duckdb/planner/filter/constant_filter.hpp"
#include "duckdb/planner/filter/null_filter.hpp"
#include "duckdb/planner/filter/conjunction_filter.hpp"
#include "duckdb/common/common.hpp"
#include "duckdb/common/exception.hpp"
#include "duckdb/common/string_util.hpp"
#include "duckdb/common/types/data_chunk.hpp"
#include "duckdb/common/multi_file_reader_options.hpp"
#include "duckdb/common/multi_file_reader.hpp"
#endif
#include "column_reader.hpp"
#include "parquet_file_metadata_cache.hpp"
#include "parquet_rle_bp_decoder.hpp"
#include "parquet_types.h"
#include "resizable_buffer.hpp"
#include "../json/json.h"
#include <exception>

namespace duckdb_parquet {
namespace format {
class FileMetaData;
}
} // namespace duckdb_parquet

namespace duckdb {
class Allocator;
class ClientContext;
class BaseStatistics;
class TableFilterSet;

struct ParquetReaderPrefetchConfig {
	// Percentage of data in a row group span that should be scanned for enabling whole group prefetch
	static constexpr double WHOLE_GROUP_PREFETCH_MINIMUM_SCAN = 0.95;
};

struct ParquetReaderScanState {
	vector<idx_t> group_idx_list;
	int64_t current_group;
	idx_t group_offset;
	unique_ptr<FileHandle> file_handle;
	unique_ptr<ColumnReader> root_reader;
	unique_ptr<duckdb_apache::thrift::protocol::TProtocol> thrift_file_proto;

	bool finished;
	SelectionVector sel;

	ResizeableBuffer define_buf;
	ResizeableBuffer repeat_buf;

	bool prefetch_mode = false;
	bool current_group_prefetched = false;
};

enum class PolicyType : uint8_t {
	INVALID = 0,
	FILTER = 1,
	OTHER = 2
};

struct Policy {
	string colName;
	PolicyType policy_type;
	ExpressionType expression_type;
	vector<unique_ptr<Policy>> child_policies;
	Value val;
	Policy(string colName, PolicyType policy_type, ExpressionType operator_type, Value &val);
	Policy(PolicyType policy_type, ExpressionType operator_type, vector<unique_ptr<Policy>> &child_policies);
	~Policy();
};

struct ParquetOptions {
	explicit ParquetOptions() {
	}
	explicit ParquetOptions(ClientContext &context);

	bool binary_as_string = false;
	bool file_row_number = false;
	string policy_file;
	int user_id;
	MultiFileReaderOptions file_options;

public:
	void Serialize(FieldWriter &writer) const;
	void Deserialize(FieldReader &reader);
};

class ParquetReader {
public:
	ParquetReader(ClientContext &context, string file_name, ParquetOptions parquet_options);
	ParquetReader(ClientContext &context, ParquetOptions parquet_options,
	              shared_ptr<ParquetFileMetadataCache> metadata);
	~ParquetReader();

	FileSystem &fs;
	Allocator &allocator;
	string file_name;
	vector<LogicalType> return_types;
	vector<string> names;
	shared_ptr<ParquetFileMetadataCache> metadata;
	ParquetOptions parquet_options;	
	vector<unique_ptr<Policy>> policies;
	Json::Value user_data;
	MultiFileReaderData reader_data;
	bool policyChecker = true;

public:
	void InitializeScan(ParquetReaderScanState &state, vector<idx_t> groups_to_read);
	void Scan(ParquetReaderScanState &state, DataChunk &output);

	idx_t NumRows();
	idx_t NumRowGroups();

	const duckdb_parquet::format::FileMetaData *GetFileMetadata();

	unique_ptr<BaseStatistics> ReadStatistics(const string &name);
	static LogicalType DeriveLogicalType(const SchemaElement &s_ele, bool binary_as_string);

	FileHandle &GetHandle() {
		return *file_handle;
	}

	const string &GetFileName() {
		return file_name;
	}
	const vector<string> &GetNames() {
		return names;
	}
	const vector<LogicalType> &GetTypes() {
		return return_types;
	}

private:
	void InitializeSchema();
	void ConstructPolicies(Json::Value &json);
	unique_ptr<Policy> ConstructConjFilter(Json::Value &filter);
	unique_ptr<Policy> ConstructConstantFilter(Json::Value &filter);
	unique_ptr<Policy> ConstructFilter(Json::Value &filter);
	void PolicyViolation(DataChunk &output);
	int ApplyPolicyFilter(vector<Vector> &v, Policy &filter, idx_t count);
	idx_t GetColIdx(string colName);
	bool ScanInternal(ParquetReaderScanState &state, DataChunk &output);
	unique_ptr<ColumnReader> CreateReader();

	unique_ptr<ColumnReader> CreateReaderRecursive(idx_t depth, idx_t max_define, idx_t max_repeat,
	                                               idx_t &next_schema_idx, idx_t &next_file_idx);
	const duckdb_parquet::format::RowGroup &GetGroup(ParquetReaderScanState &state);
	uint64_t GetGroupCompressedSize(ParquetReaderScanState &state);
	idx_t GetGroupOffset(ParquetReaderScanState &state);
	// Group span is the distance between the min page offset and the max page offset plus the max page compressed size
	uint64_t GetGroupSpan(ParquetReaderScanState &state);
	void PrepareRowGroupBuffer(ParquetReaderScanState &state, idx_t out_col_idx);
	LogicalType DeriveLogicalType(const SchemaElement &s_ele);

	template <typename... Args>
	std::runtime_error FormatException(const string fmt_str, Args... params) {
		return std::runtime_error("Failed to read Parquet file \"" + file_name +
		                          "\": " + StringUtil::Format(fmt_str, params...));
	}

private:
	unique_ptr<FileHandle> file_handle;
};

} // namespace duckdb
