#pragma once

#include <algorithm>
#include <functional>
#include <numeric>
#include <unordered_map>
#include <utility>
#include <vector>
#include "common/macros.h"
#include "storage/index/index_defs.h"
#include "storage/projected_row.h"
#include "storage/storage_util.h"
#include "type/type_util.h"

namespace terrier::storage::index {

/**
 * Precomputes index-related metadata that can be used to optimize the operations of the various index key types.
 */
class IndexMetadata {
 public:
  DISALLOW_COPY(IndexMetadata);

  IndexMetadata(IndexMetadata &&other) noexcept
      : key_schema_(std::move(other.key_schema_)),
        attr_sizes_(std::move(other.attr_sizes_)),
        compact_ints_offsets_(std::move(other.compact_ints_offsets_)),
        key_oid_to_offset_(std::move(other.key_oid_to_offset_)),
        initializer_(std::move(other.initializer_)) {}

  /**
   * Precomputes metadata for the given key schema.
   * @param key_schema index key schema
   */
  explicit IndexMetadata(IndexKeySchema key_schema)
      : key_schema_(std::move(key_schema)),
        attr_sizes_(ComputeAttributeSizes(key_schema_)),
        compact_ints_offsets_(ComputeCompactIntsOffsets(attr_sizes_)),
        key_oid_to_offset_(ComputeKeyOidToOffset(key_schema_, ComputePROffsets(attr_sizes_))),
        initializer_(ProjectedRowInitializer::CreateProjectedRowInitializerForIndexes(
            attr_sizes_, ComputeComparisonOrder(attr_sizes_))) {}

  /**
   * @return index key schema
   */
  const std::vector<IndexKeyColumn> &GetKeySchema() const { return key_schema_; }

  /**
   * @return unsorted index attribute sizes (key schema order)
   */
  const std::vector<uint8_t> &GetAttributeSizes() const { return attr_sizes_; }

  /**
   * @return offsets to write into for compact ints (key schema order)
   */
  const std::vector<uint8_t> &GetCompactIntsOffsets() const { return compact_ints_offsets_; }

  /**
   * @return mapping from key oid to projected row offset
   */
  const std::unordered_map<catalog::indexkeycol_oid_t, uint32_t> &GetKeyOidToOffsetMap() const {
    return key_oid_to_offset_;
  }

  /**
   * @return projected row initializer for the given key schema
   */
  const ProjectedRowInitializer &GetProjectedRowInitializer() const { return initializer_; }

 private:
  std::vector<IndexKeyColumn> key_schema_;                                      // for GenericKey
  std::vector<uint8_t> attr_sizes_;                                             // for CompactIntsKey
  std::vector<uint8_t> compact_ints_offsets_;                                   // for CompactIntsKey
  std::unordered_map<catalog::indexkeycol_oid_t, uint32_t> key_oid_to_offset_;  // for execution layer
  ProjectedRowInitializer initializer_;

  /**
   * Computes the attribute sizes as given by the key schema.
   * e.g.   if key_schema is {INTEGER, INTEGER, BIGINT, TINYINT, SMALLINT}
   *        then attr_sizes returned is {4, 4, 8, 1, 2}
   */
  static std::vector<uint8_t> ComputeAttributeSizes(const IndexKeySchema &key_schema) {
    std::vector<uint8_t> attr_sizes;
    attr_sizes.reserve(key_schema.size());
    for (const auto &key : key_schema) {
      attr_sizes.emplace_back(type::TypeUtil::GetTypeSize(key.type_id));
    }
    return attr_sizes;
  }

  /**
   * Computes the final comparison order for the given vector of attribute sizes.
   * e.g.   if attr_sizes {4, 4, 8, 1, 2},
   *        begin cmp_order is always assumed to be {0, 1, 2, 3, 4},
   *        final cmp_order is then {2, 0, 1, 4, 3}.
   */
  static std::vector<uint16_t> ComputeComparisonOrder(const std::vector<uint8_t> &attr_sizes) {
    // note: at most uint16_t num_columns in ProjectedRow
    std::vector<uint16_t> cmp_order(attr_sizes.size());
    std::iota(cmp_order.begin(), cmp_order.end(), 0);
    std::stable_sort(cmp_order.begin(), cmp_order.end(),
                     [&](const uint16_t &i, const uint16_t &j) -> bool { return attr_sizes[i] > attr_sizes[j]; });
    return cmp_order;
  }

  /**
   * Computes the compact int offsets for the given vector of attribute sizes.
   * e.g.   if attr_sizes {4, 4, 8, 1, 2}
   *        exclusive scan {0, 4, 8, 16, 17}
   *        gives where you should write the attrs in a compact ints key
   */
  static std::vector<uint8_t> ComputeCompactIntsOffsets(const std::vector<uint8_t> &attr_sizes) {
    // exclusive scan on a copy
    std::vector<uint8_t> scan = attr_sizes;
    std::exclusive_scan(scan.begin(), scan.end(), scan.begin(), 0u);
    return scan;
  }

  /**
   * Computes the projected row offsets given the attribute sizes, i.e. where they end up after reshuffling.
   * e.g.   if attr_sizes is {4, 4, 8, 1, 2}
   *        then returned offsets are {1, 2, 0, 4, 3}
   */
  static std::vector<uint16_t> ComputePROffsets(const std::vector<uint8_t> &attr_sizes) {
    auto starting_offsets = StorageUtil::ComputeBaseAttributeOffsets(attr_sizes, 0);

    std::vector<uint16_t> pr_offsets;
    pr_offsets.reserve(attr_sizes.size());

    for (const auto &size : attr_sizes) {
      switch (size) {
        case VARLEN_COLUMN:
          pr_offsets.emplace_back(starting_offsets[0]++);
          break;
        case 8:
          pr_offsets.emplace_back(starting_offsets[1]++);
          break;
        case 4:
          pr_offsets.emplace_back(starting_offsets[2]++);
          break;
        case 2:
          pr_offsets.emplace_back(starting_offsets[3]++);
          break;
        case 1:
          pr_offsets.emplace_back(starting_offsets[4]++);
          break;
        default:
          throw std::runtime_error("unexpected switch case value");
      }
    }

    return pr_offsets;
  }

  /**
   * Computes the mapping from key oid to projected row offset.
   */
  static std::unordered_map<catalog::indexkeycol_oid_t, uint32_t> ComputeKeyOidToOffset(
      const IndexKeySchema &key_schema, const std::vector<uint16_t> &pr_offsets) {
    std::unordered_map<catalog::indexkeycol_oid_t, uint32_t> key_oid_to_offset;
    key_oid_to_offset.reserve(key_schema.size());
    for (uint16_t i = 0; i < key_schema.size(); i++) {
      key_oid_to_offset[key_schema[i].indexkeycol_oid] = pr_offsets[i];
    }
    return key_oid_to_offset;
  }
};

}  // namespace terrier::storage::index
