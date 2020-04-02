#pragma once
#include <list>
#include <unordered_map>
#include <vector>

#include "common/performance_counter.h"
#include "common/shared_latch.h"
#include "storage/projected_columns.h"
#include "storage/storage_defs.h"
#include "storage/tuple_access_strategy.h"
#include "storage/undo_record.h"

namespace terrier::transaction {
class TransactionContext;
class TransactionManager;
}  // namespace terrier::transaction

namespace terrier::storage {

namespace index {
class Index;
template <typename KeyType>
class BwTreeIndex;
template <typename KeyType>
class HashIndex;
}  // namespace index

// clang-format off
#define DataTableCounterMembers(f) \
  f(uint64_t, NumSelect) \
  f(uint64_t, NumUpdate) \
  f(uint64_t, NumInsert) \
  f(uint64_t, NumDelete) \
  f(uint64_t, NumNewBlock)
// clang-format on
DEFINE_PERFORMANCE_CLASS(DataTableCounter, DataTableCounterMembers)
#undef DataTableCounterMembers

/**
 * A DataTable is a thin layer above blocks that handles visibility, schemas, and maintenance of versions for a
 * SQL table. This class should be the main outward facing API for the storage engine. SQL level concepts such
 * as SQL types, varlens and nullabilities are still not meaningful at this level.
 */
class DataTable {
 public:
  /**
   * Iterator for all the slots, claimed or otherwise, in the data table. This is useful for sequential scans.
   */
  class SlotIterator {
   public:
    /**
     * @return reference to the underlying tuple slot
     */
    TupleSlot &operator*() {
      uint64_t block_num = i_ / max_slots_;
      RawBlock *b = table_->array_[block_num].load();
      current_slot_ = {b, static_cast<uint32_t>(i_ % static_cast<uint64_t>(max_slots_))};
      return current_slot_;
    }

    /**
     * @return pointer to the underlying tuple slot
     */
    TupleSlot *operator->() {
      operator*();
      return &current_slot_;
    }

    /**
     * pre-fix increment.
     * @return self-reference after the iterator is advanced
     */
    SlotIterator &operator++();

    /**
     * post-fix increment.
     * @return copy of the iterator equal to this before increment
     */
    SlotIterator operator++(int) {
      SlotIterator copy = *this;
      operator++();
      return copy;
    }

    /**
     * Equality check.
     * @param other other iterator to compare to
     * @return if the two iterators point to the same slot
     */
    bool operator==(const SlotIterator &other) const {
      // TODO(Tianyu): I believe this is enough?
      if (other.is_end_) {
        return i_ >= max_iters_;
      }
      if (is_end_) {
        return other.i_ >= other.max_iters_;
      }
      return table_ == other.table_ && i_ == other.i_;
    }

    /**
     * Inequality check.
     * @param other other iterator to compare to
     * @return if the two iterators are not equal
     */
    bool operator!=(const SlotIterator &other) const { return !this->operator==(other); }

   private:
    friend class DataTable;
    /**
     * @warning MUST BE CALLED ONLY WHEN CALLER HOLDS LOCK TO THE LIST OF RAW BLOCKS IN THE DATA TABLE
     */
    SlotIterator(const DataTable *table, bool is_end)
        : table_(table),
          max_iters_(0),
          i_(0),
          max_slots_(table_->accessor_.GetBlockLayout().NumSlots()),
          is_end_(is_end) {
      if (is_end_) {
        return;
      }
      uint64_t current_size = table_->write_num_.load();
      RawBlock *last_block = table_->array_[current_size - 1].load();
      max_iters_ = (current_size - 1) * max_slots_ + (*last_block).GetInsertHead();
    }

    // TODO(Tianyu): Can potentially collapse this information into the RawBlock so we don't have to hold a pointer to
    // the table anymore. Right now we need the table to know how many slots there are in the block
    const DataTable *table_;
    uint64_t max_iters_;
    uint64_t i_;
    uint32_t max_slots_;
    bool is_end_;
    TupleSlot current_slot_;
  };
  /**
   * Constructs a new DataTable with the given layout, using the given BlockStore as the source
   * of its storage blocks. The first column must be size 8 and is effectively hidden from upper levels.
   *
   * @param store the Block store to use.
   * @param layout the initial layout of this DataTable. First 2 columns must be 8 bytes.
   * @param layout_version the layout version of this DataTable
   */
  DataTable(common::ManagedPointer<BlockStore> const store, const BlockLayout &layout, layout_version_t layout_version);

  /**
   * Destructs a DataTable, frees all its blocks and any potential varlen entries.
   */
  ~DataTable();

  /**
   * Materializes a single tuple from the given slot, as visible to the transaction given, according to the format
   * described by the given output buffer.
   *
   * @param txn the calling transaction
   * @param slot the tuple slot to read
   * @param out_buffer output buffer. The object should already contain projection list information and should not
   * reference col_id 0
   * @return true if tuple is visible to this txn and ProjectedRow has been populated, false otherwise
   */
  bool Select(common::ManagedPointer<transaction::TransactionContext> txn, TupleSlot slot,
              ProjectedRow *out_buffer) const;

  // TODO(Tianyu): Should this be updated in place or return a new iterator? Does the caller ever want to
  // save a point of scan and come back to it later?
  // Alternatively, we can provide an easy wrapper that takes in a const SlotIterator & and returns a SlotIterator,
  // just like the ++i and i++ dichotomy.
  /**
   * Sequentially scans the table starting from the given iterator(inclusive) and materializes as many tuples as would
   * fit into the given buffer, as visible to the transaction given, according to the format described by the given
   * output buffer. The tuples materialized are guaranteed to be visible and valid, and the function makes best effort
   * to fill the buffer, unless there are no more tuples. The given iterator is mutated to point to one slot passed the
   * last slot scanned in the invocation.
   *
   * @param txn the calling transaction
   * @param start_pos iterator to the starting location for the sequential scan
   * @param out_buffer output buffer. The object should already contain projection list information. This buffer is
   *                   always cleared of old values.
   */
  void Scan(common::ManagedPointer<transaction::TransactionContext> txn, SlotIterator *start_pos,
            ProjectedColumns *out_buffer) const;

  /**
   * @return the first tuple slot contained in the data table
   */
  SlotIterator begin() const {  // NOLINT for STL name compability
    return {this, false};
  }

  /**
   * Returns one past the last tuple slot contained in the data table. Note that this is not an accurate number when
   * concurrent accesses are happening, as inserts maybe in flight. However, the number given is always transactionally
   * correct, as any inserts that might have happened is not going to be visible to the calling transaction.
   *
   * @return one past the last tuple slot contained in the data table.
   */
  SlotIterator end() const;  // NOLINT for STL name compability

  /**
   * Update the tuple according to the redo buffer given, and update the version chain to link to an
   * undo record that is allocated in the txn. The undo record is populated with a before-image of the tuple in the
   * process. Update will only happen if there is no write-write conflict and tuple is visible, otherwise, this is
   * equivalent to a noop and false is returned. If return is false, undo's table pointer is nullptr (used in Abort and
   * GC)
   *
   * @param txn the calling transaction
   * @param slot the slot of the tuple to update.
   * @param redo the desired change to be applied. This should be the after-image of the attributes of interest. Should
   * not reference col_id 0
   * @return true if successful, false otherwise
   */
  bool Update(common::ManagedPointer<transaction::TransactionContext> txn, TupleSlot slot, const ProjectedRow &redo);

  /**
   * Inserts a tuple, as given in the redo, and update the version chain the link to the given
   * delta record. The slot allocated for the tuple is returned.
   *
   * @param txn the calling transaction
   * @param redo after-image of the inserted tuple. Should not reference col_id 0
   * @return the TupleSlot allocated for this insert, used to identify this tuple's physical location for indexes and
   * such.
   */
  TupleSlot Insert(common::ManagedPointer<transaction::TransactionContext> txn, const ProjectedRow &redo);

  /**
   * Deletes the given TupleSlot, this will call StageDelete on the provided txn to generate the RedoRecord for delete.
   * The rest of the behavior follows Update's behavior.
   * @param txn the calling transaction
   * @param slot the slot of the tuple to delete
   * @return true if successful, false otherwise
   */
  bool Delete(common::ManagedPointer<transaction::TransactionContext> txn, TupleSlot slot);

  /**
   * Return a pointer to the performance counter for the data table.
   * @return pointer to the performance counter
   */
  DataTableCounter *GetDataTableCounter() { return &data_table_counter_; }

  /**
   * @return read-only view of this DataTable's BlockLayout
   */
  const BlockLayout &GetBlockLayout() const { return accessor_.GetBlockLayout(); }

  /**
   * @return this DataTable's Accessor
   */
  const TupleAccessStrategy *GetAccessor() const { return &accessor_; }

  /**
   * @return a pointer to this DataTable's array of RawBlock*
   */
  const std::atomic<std::atomic<RawBlock *> *> *GetBlockArray() const { return &array_; }

  /**
   * @return the number of blocks that are in the RawBlock* array
   */
  uint64_t GetNumBlocks() const { return write_num_; }

 private:
  // The GarbageCollector needs to modify VersionPtrs when pruning version chains
  friend class GarbageCollector;
  // The TransactionManager needs to modify VersionPtrs when rolling back aborts
  friend class transaction::TransactionManager;
  // The index wrappers need access to IsVisible and HasConflict
  friend class index::Index;
  template <typename KeyType>
  friend class index::BwTreeIndex;
  template <typename KeyType>
  friend class index::HashIndex;
  // The block compactor elides transactional protection in the gather/compression phase and
  // needs raw access to the underlying table.
  friend class BlockCompactor;

  common::ManagedPointer<BlockStore> const block_store_;
  const layout_version_t layout_version_;
  const TupleAccessStrategy accessor_;
  const uint64_t array_start_size_ = 256;
  const uint64_t array_resize_factor_ = 2;
  const SlotIterator end_ = {this, true};

  // TODO(Tianyu): For now, on insertion, we simply sequentially go through a block and allocate a
  // new one when the current one is full. Needless to say, we will need to revisit this when extending GC to handle
  // deleted tuples and recycle slots
  // TODO(Tianyu): Now that we are switching to a linked list, there probably isn't a reason for it
  // to be latched. Could just easily write a lock-free one if there's performance gain(probably not). vector->list has
  // negligible difference in insert performance (within margin of error) when benchmarked.
  // We also might need our own implementation because we need to handle GC of an unlinked block, as a sequential scan
  // might be on it
  std::atomic_uint64_t offset_, insert_index_, array_ref_counter_, size_, write_num_;
  std::atomic<std::atomic<RawBlock *> *> array_;

  std::atomic_bool resizing_;
  std::mutex resizing_mux_;
  std::condition_variable done_resizing_;
  // Check if we need to advance the insertion_head_
  // This function uses header_latch_ to ensure correctness
  mutable DataTableCounter data_table_counter_;

  // A templatized version for select, so that we can use the same code for both row and column access.
  // the method is explicitly instantiated for ProjectedRow and ProjectedColumns::RowView
  template <class RowType>
  bool SelectIntoBuffer(common::ManagedPointer<transaction::TransactionContext> txn, TupleSlot slot,
                        RowType *out_buffer) const;

  void InsertInto(common::ManagedPointer<transaction::TransactionContext> txn, const ProjectedRow &redo,
                  TupleSlot dest);
  // Atomically read out the version pointer value.
  UndoRecord *AtomicallyReadVersionPtr(TupleSlot slot, const TupleAccessStrategy &accessor) const;

  // Atomically write the version pointer value. Should only be used by Insert where there is guaranteed to be no
  // contention
  void AtomicallyWriteVersionPtr(TupleSlot slot, const TupleAccessStrategy &accessor, UndoRecord *desired);

  // Checks for Snapshot Isolation conflicts, used by Update
  bool HasConflict(const transaction::TransactionContext &txn, UndoRecord *version_ptr) const;

  // Wrapper around the other HasConflict for indexes to call (they only have tuple slot, not the version pointer)
  bool HasConflict(const transaction::TransactionContext &txn, TupleSlot slot) const;

  // Performs a visibility check on the designated TupleSlot. Note that this does not traverse a version chain, so this
  // information alone is not enough to determine visibility of a tuple to a transaction. This should be used along with
  // a version chain traversal to determine if a tuple's versions are actually visible to a txn.
  // The criteria for visibility of a slot are presence (slot is occupied) and not deleted
  // (logical delete bitmap is non-NULL).
  bool Visible(TupleSlot slot, const TupleAccessStrategy &accessor) const;

  // Compares and swaps the version pointer to be the undo record, only if its value is equal to the expected one.
  bool CompareAndSwapVersionPtr(TupleSlot slot, const TupleAccessStrategy &accessor, UndoRecord *expected,
                                UndoRecord *desired);

  // Allocates a new block to be used as insertion head.
  RawBlock *NewBlock();

  /**
   * Determine if a Tuple is visible (present and not deleted) to the given transaction. It's effectively Select's logic
   * (follow a version chain if present) without the materialization. If the logic of Select changes, this should change
   * with it and vice versa.
   * @param txn the calling transaction
   * @param slot the slot of the tuple to check visibility on
   * @return true if tuple is visible to this txn, false otherwise
   */
  bool IsVisible(const transaction::TransactionContext &txn, TupleSlot slot) const;
};
}  // namespace terrier::storage
