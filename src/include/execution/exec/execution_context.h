#pragma once
#include <memory>
#include <utility>
#include "catalog/catalog_accessor.h"
#include "execution/exec/output.h"
#include "execution/sql/memory_pool.h"
#include "execution/sql/memory_tracker.h"
#include "execution/util/region.h"
#include "planner/plannodes/output_schema.h"
#include "transaction/transaction_context.h"
#include "transaction/transaction_manager.h"

namespace terrier::execution::exec {
/**
 * Execution Context: Stores information handed in by upper layers.
 * TODO(Amadou): This class will change once we know exactly what we get from upper layers.
 */
class EXPORT ExecutionContext {
 public:
  /**
   * An allocator for short-ish strings. Needed because the requirements of
   * string allocation (small size and frequent usage) are slightly different
   * than that of generic memory allocator for larger structures. This string
   * allocator relies on memory regions for fast allocations, and bulk
   * deletions.
   */
  class StringAllocator {
   public:
    /**
     * Create a new allocator
     */
    StringAllocator(common::ManagedPointer<sql::MemoryTracker> tracker) : region_(""), tracker_(tracker) {}

    /**
     * This class cannot be copied or moved.
     */
    DISALLOW_COPY_AND_MOVE(StringAllocator);

    /**
     * Destroy allocator
     */
    ~StringAllocator() = default;

    /**
     * Allocate a string of the given size..
     * @param size Size of the string in bytes.
     * @return A pointer to the string.
     */
    char *Allocate(std::size_t size);

    /**
     * No-op. Bulk de-allocated upon destruction.
     */
    void Deallocate(char *str) {}

   private:
    util::Region region_;
    // Metadata tracker for memory allocations
    common::ManagedPointer<sql::MemoryTracker> tracker_;
  };

  /**
   * Constructor
   * @param db_oid oid of the database
   * @param txn transaction used by this query
   * @param callback callback function for outputting
   * @param schema the schema of the output
   * @param accessor the catalog accessor of this query
   */
  ExecutionContext(catalog::db_oid_t db_oid, transaction::TransactionContext *txn, const OutputCallback &callback,
                   const planner::OutputSchema *schema, std::unique_ptr<catalog::CatalogAccessor> &&accessor)
      : db_oid_(db_oid),
        txn_(txn),
        mem_tracker_(std::make_unique<sql::MemoryTracker>()),
        mem_pool_(std::make_unique<sql::MemoryPool>(common::ManagedPointer<sql::MemoryTracker>(mem_tracker_))),
        buffer_(schema == nullptr ? nullptr
                                  : std::make_unique<OutputBuffer>(mem_pool_.get(), schema->GetColumns().size(),
                                                                   ComputeTupleSize(schema), callback)),
        string_allocator_(common::ManagedPointer<sql::MemoryTracker>(mem_tracker_)),
        accessor_(std::move(accessor)) {}

  /**
   * @return the transaction used by this query
   */
  transaction::TransactionContext *GetTxn() { return txn_; }

  /**
   * @return the output buffer used by this query
   */
  OutputBuffer *GetOutputBuffer() { return buffer_.get(); }

  /**
   * @return the memory pool
   */
  sql::MemoryPool *GetMemoryPool() { return mem_pool_.get(); }

  /**
   * @return the string allocator
   */
  StringAllocator *GetStringAllocator() { return &string_allocator_; }

  /**
   * @param schema the schema of the output
   * @return the size of tuple with this final_schema
   */
  static uint32_t ComputeTupleSize(const planner::OutputSchema *schema);

  /**
   * @return the catalog accessor
   */
  catalog::CatalogAccessor *GetAccessor() { return accessor_.get(); }

  /**
   * Start the resource tracker
   */
  void StartResourceTracker();

  /**
   * End the resource tracker and record the metrics
   * @param name the string name get printed out with the time
   * @param len the length of the string name
   */
  void EndResourceTracker(const char *name, uint32_t len);

  /**
   * @return the db oid
   */
  catalog::db_oid_t DBOid() { return db_oid_; }

  /**
   * Set the mode for this execution.
   * This only records the mode and serves the metrics collection purpose, which does not have any impact on the
   * actual execution.
   * @param mode the integer value of the execution mode to record
   */
  void SetExecutionMode(uint8_t mode) { execution_mode_ = mode; }

 private:
  catalog::db_oid_t db_oid_;
  transaction::TransactionContext *txn_;
  std::unique_ptr<sql::MemoryTracker> mem_tracker_;
  std::unique_ptr<sql::MemoryPool> mem_pool_;
  std::unique_ptr<OutputBuffer> buffer_;
  StringAllocator string_allocator_;
  std::unique_ptr<catalog::CatalogAccessor> accessor_;
  uint8_t execution_mode_;
};
}  // namespace terrier::execution::exec
