#include "duckdb/execution/operator/helper/physical_materialized_collector.hpp"
#include "duckdb/common/types/chunk_collection.hpp"
#include "duckdb/main/materialized_query_result.hpp"
#include "duckdb/main/client_context.hpp"
#include <iostream>
namespace duckdb {

PhysicalMaterializedCollector::PhysicalMaterializedCollector(PreparedStatementData &data, bool parallel)
    : PhysicalResultCollector(data), parallel(parallel) {
	collectAllCheckers(plan);
}

void PhysicalMaterializedCollector::runInputCheckers(StateStore &store) const {
	for(auto &inputCheckerFunc: inputCheckers) {
		inputCheckerFunc(store);
	}
	return;
}

void PhysicalMaterializedCollector::collectAllCheckers(	PhysicalOperator &plan ) {
	for(auto &checker: plan.inputCheckers) {
		this->inputCheckers.emplace_back(checker);
	}

	for(auto &child: plan.children) {
		collectAllCheckers(*child);
	}

	return;
}
//===--------------------------------------------------------------------===//
// Sink
//===--------------------------------------------------------------------===//
class MaterializedCollectorGlobalState : public GlobalSinkState {
public:
	mutex glock;
	unique_ptr<ColumnDataCollection> collection;
	shared_ptr<ClientContext> context;
	StateStore *final_gstore = new StateStore();
};

class MaterializedCollectorLocalState : public LocalSinkState {
public:
	unique_ptr<ColumnDataCollection> collection;
	ColumnDataAppendState append_state;
	StateStore *final_lstore = new StateStore();
};

SinkResultType PhysicalMaterializedCollector::Sink(ExecutionContext &context, DataChunk &chunk,
                                                   OperatorSinkInput &input) const {
	auto &lstate = input.local_state.Cast<MaterializedCollectorLocalState>();
	lstate.collection->Append(lstate.append_state, chunk);
	lstate.final_lstore->MergeStore(*chunk.store);
	return SinkResultType::NEED_MORE_INPUT;
}

void PhysicalMaterializedCollector::Combine(ExecutionContext &context, GlobalSinkState &gstate_p,
                                            LocalSinkState &lstate_p) const {
	auto &gstate = gstate_p.Cast<MaterializedCollectorGlobalState>();
	auto &lstate = lstate_p.Cast<MaterializedCollectorLocalState>();

	gstate.final_gstore->MergeStore(*lstate.final_lstore);

	if (lstate.collection->Count() == 0) {
		return;
	}

	lock_guard<mutex> l(gstate.glock);
	if (!gstate.collection) {
		gstate.collection = std::move(lstate.collection);
	} else {
		gstate.collection->Combine(*lstate.collection);
	}
}

unique_ptr<GlobalSinkState> PhysicalMaterializedCollector::GetGlobalSinkState(ClientContext &context) const {
	auto state = make_uniq<MaterializedCollectorGlobalState>();
	state->context = context.shared_from_this();
	return std::move(state);
}

unique_ptr<LocalSinkState> PhysicalMaterializedCollector::GetLocalSinkState(ExecutionContext &context) const {
	auto state = make_uniq<MaterializedCollectorLocalState>();
	state->collection = make_uniq<ColumnDataCollection>(Allocator::DefaultAllocator(), types);
	state->collection->InitializeAppend(state->append_state);
	return std::move(state);
}

unique_ptr<QueryResult> PhysicalMaterializedCollector::GetResult(GlobalSinkState &state) {
	auto &gstate = state.Cast<MaterializedCollectorGlobalState>();

	runInputCheckers(*gstate.final_gstore);

	if (!gstate.collection) {
		gstate.collection = make_uniq<ColumnDataCollection>(Allocator::DefaultAllocator(), types);
	}
	auto result = make_uniq<MaterializedQueryResult>(statement_type, properties, names, std::move(gstate.collection),
	                                                 gstate.context->GetClientProperties());
	return std::move(result);
}

bool PhysicalMaterializedCollector::ParallelSink() const {
	return parallel;
}

bool PhysicalMaterializedCollector::SinkOrderDependent() const {
	return true;
}

} // namespace duckdb
