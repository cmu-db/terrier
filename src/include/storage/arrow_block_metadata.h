#pragma once
#include <map>
#include <unordered_set>
#include "storage/block_layout.h"
#include "storage/storage_defs.h"
#include "storage/storage_util.h"

namespace terrier::storage {

// TODO(Tianyu): Can use to specify cases where we don't concat per-block in the future (e.g. no need to put
//  blob into Arrow)
/**
 * Type of Arrow column
 */
enum class ArrowColumnType : uint8_t { FIXED_LENGTH = 0, GATHERED_VARLEN, DICTIONARY_COMPRESSED };

/**
 * Stores information about an Arrow varlen column. This class implements an Arrow list, with
 * a byte array of values and an array of offsets into the value array
 */
class ArrowVarlenColumn {
 public:
  /**
   * Constructs an empty Arrow VarlenColumn
   */
  ArrowVarlenColumn() = default;

  /**
   * Constructs a new arrow varlen column object
   * @param values_length size of the values buffer, in bytes
   * @param offsets_length number of elements in the offsets array
   */
  ArrowVarlenColumn(uint32_t values_length, uint32_t offsets_length)
      : values_length_(values_length),
        offsets_length_(offsets_length),
        values_(common::AllocationUtil::AllocateAligned(values_length_)),
        offsets_(common::AllocationUtil::AllocateAligned<uint32_t>(offsets_length_)) {}

  DISALLOW_COPY(ArrowVarlenColumn)

  /**
   * Move constructor
   * @param other object to move from
   */
  ArrowVarlenColumn(ArrowVarlenColumn &&other) noexcept
      : values_length_(other.values_length_),
        offsets_length_(other.offsets_length_),
        values_(other.values_),
        offsets_(other.offsets_) {
    other.values_ = nullptr;
    other.offsets_ = nullptr;
  }

  /**
   * Move-assigmenet operator
   * @param other object to move from
   * @return self-reference
   */
  ArrowVarlenColumn &operator=(ArrowVarlenColumn &&other) noexcept {
    if (this != &other) {
      // check self-assignment
      values_length_ = other.values_length_;
      offsets_length_ = other.offsets_length_;
      delete[] values_;
      values_ = other.values_;
      other.values_ = nullptr;
      delete[] offsets_;
      offsets_ = other.offsets_;
      other.offsets_ = nullptr;
    }
    return *this;
  }

  /**
   * Destructs an ArrowVarlenColumn
   */
  ~ArrowVarlenColumn() {
    delete[] values_;
    delete[] offsets_;
  }

  /**
   * @return length of the values array
   */
  uint32_t ValuesLength() const { return values_length_; }

  /**
   * @return length of the offsets array
   */
  uint32_t getOffsetsLength() const { return offsets_length_; }

  /**
   * @return the values array
   */
  byte *getValues() const { return values_; }

  /**
   * @return the offsets array
   */
  uint32_t *getOffsets() const { return offsets_; }

 private:
  uint32_t values_length_ = 0, offsets_length_ = 0;
  byte *values_ = nullptr;
  uint32_t *offsets_ = nullptr;
};

/**
 * Stores information about accessing a column using the Arrow format. This includes the type of the column,
 * and pointers to various buffers if the column is variable length or compressed.
 */
class ArrowColumnInfo {
 public:
  ~ArrowColumnInfo() { delete[] indices_; }

  /**
   * @return type of the Arrow Column
   */
  ArrowColumnType Type() const { return type_; }
  /**
   * @return ArrowVarlenColumn object for the column
   */
  ArrowVarlenColumn &VarlenColumn() { return varlen_column_; }

  /**
   * Returns the indices array. This array is only meaningful if the column is dictionary compressed. The
   * size of this array is equal to the number of slots in a block.
   * @return the indices array
   */
  uint32_t *getIndices() const {
    TERRIER_ASSERT(type_ == ArrowColumnType::DICTIONARY_COMPRESSED,
                   "this array is only meaningful if the column is dicationary compressed");
    return indices_;
  }

 private:
  /**
   * type of this Arrow column
   */
  ArrowColumnType type_;
  ArrowVarlenColumn varlen_column_;  // For varlen and dictionary
  // TODO(Tianyu): Add null bitmap
  uint32_t *indices_ = nullptr;  // for dictionary
};

/**
 * This class encapsulates all the information needed by arrow to interpret a block, such as
 * length, null counts, and the start of varlen columns, etc. (non varlen columns start can be
 * computed from block start and offset for that column, as they are essentially embedded in the
 * block itself)
 *
 * Notice that the information stored in the metadata maybe outdated if the block is hot. (Things like
 * null counts are not well defined independent of a transaction when the block is versioned)
 */
class ArrowBlockMetadata {
 public:
  MEM_REINTERPRETATION_ONLY(ArrowBlockMetadata)

  /**
   * @param num_cols number of columns stored in the block
   * @return size of the metadata object given the number of columns
   */
  static uint32_t Size(uint16_t num_cols) {
    return StorageUtil::PadUpToSize(sizeof(uint64_t), static_cast<uint32_t>(sizeof(uint32_t)) * (num_cols + 1)) +
           num_cols * static_cast<uint32_t>(sizeof(ArrowColumnInfo));
  }

  /**
   * Zeroes out the memory chunk for block metadata
   * @param num_cols number of columsn stored in the block
   */
  void Initialize(uint16_t num_cols) {
    // Need to 0 out this block to make sure all the counts are 0 and all the pointers are nullptrs
    memset(this, 0, Size(num_cols));
  }

  /**
   * @return reference to the number of records value
   */
  uint32_t &NumRecords() { return num_records_; }

  /**
   * @return number of records in the block
   */
  uint32_t NumRecords() const { return num_records_; }

  /**
   *
   * @param col_id the column of interest
   * @return reference to the null count value for given column
   */
  uint32_t &NullCount(col_id_t col_id) { return reinterpret_cast<uint32_t *>(varlen_content_)[!col_id]; }

  /**
   * @param col_id the column of interest
   * @return the null count for given column
   */
  uint32_t NullCount(col_id_t col_id) const { return reinterpret_cast<const uint32_t *>(varlen_content_)[!col_id]; }

  /**
   * @param layout layout object of the Block
   * @param col_id the column of interest
   * @return ArrowColumnInfo object of the given column
   */
  ArrowColumnInfo &GetColumnInfo(const BlockLayout &layout, col_id_t col_id) {
    byte *null_count_end =
        storage::StorageUtil::AlignedPtr(sizeof(uint64_t), varlen_content_ + sizeof(uint32_t) * layout.NumColumns());
    return reinterpret_cast<ArrowColumnInfo *>(null_count_end)[!col_id];
  }

  /**
   * @param layout layout object of the Block
   * @param col_id the column of interest
   * @return ArrowColumnInfo object of the given column
   */
  const ArrowColumnInfo &GetColumnInfo(const BlockLayout &layout, col_id_t col_id) const {
    byte *null_count_end =
        storage::StorageUtil::AlignedPtr(sizeof(uint64_t), varlen_content_ + sizeof(uint32_t) * layout.NumColumns());
    return reinterpret_cast<ArrowColumnInfo *>(null_count_end)[!col_id];
  }

 private:
  uint32_t num_records_;  // number of actual records
  // null_count[num_cols] (32-bit) | padding up to 8 byte-aligned | arrow_varlen_buffers[num_cols] |
  byte varlen_content_[];
};
}  // namespace terrier::storage
