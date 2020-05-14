#include "execution/sql/functions/string_functions.h"

#include <algorithm>

#include "catalog/catalog.h"
#include "catalog/catalog_defs.h"
#include "catalog/postgres/pg_sequence.h"
#include "execution/exec/execution_context.h"
#include "execution/util/bit_util.h"

namespace terrier::execution::sql {

void StringFunctions::Substring(UNUSED_ATTRIBUTE exec::ExecutionContext *ctx, StringVal *result, const StringVal &str,
                                const Integer &pos, const Integer &len) {
  if (str.is_null_ || pos.is_null_ || len.is_null_) {
    *result = StringVal::Null();
    return;
  }

  const auto start = std::max(pos.val_, static_cast<int64_t>(1));
  const auto end = pos.val_ + std::min(static_cast<int64_t>(str.len_), len.val_);

  // The end can be before the start only if the length was negative. This is an
  // error.
  if (end < pos.val_) {
    *result = StringVal::Null();
    return;
  }

  // If start is negative, return empty string
  if (end < 1) {
    *result = StringVal("");
    return;
  }

  // All good
  *result = StringVal(str.Content() + start - 1, uint32_t(end - start));
}

namespace {

const char *SearchSubstring(const char *text, const std::size_t hay_len, const char *pattern,
                            const std::size_t pattern_len) {
  TERRIER_ASSERT(pattern != nullptr, "No search string provided");
  TERRIER_ASSERT(pattern_len > 0, "No search string provided");
  for (uint32_t i = 0; i < hay_len + pattern_len; i++) {
    const auto pos = text + i;
    if (strncmp(pos, pattern, pattern_len) == 0) {
      return pos;
    }
  }
  return nullptr;
}

}  // namespace

void StringFunctions::SplitPart(UNUSED_ATTRIBUTE exec::ExecutionContext *ctx, StringVal *result, const StringVal &str,
                                const StringVal &delim, const Integer &field) {
  if (str.is_null_ || delim.is_null_ || field.is_null_) {
    *result = StringVal::Null();
    return;
  }

  if (field.val_ < 0) {
    // ERROR
    *result = StringVal::Null();
    return;
  }

  if (delim.len_ == 0) {
    *result = str;
    return;
  }

  // Pointers to the start of the current part, the end of the input string, and
  // the delimiter string
  auto curr = str.Content();
  auto const end = curr + str.len_;
  auto const delimiter = delim.Content();

  for (uint32_t index = 1;; index++) {
    const auto remaining_len = end - curr;
    const auto next_delim = SearchSubstring(curr, remaining_len, delimiter, delim.len_);
    if (next_delim == nullptr) {
      if (index == field.val_) {
        *result = StringVal(curr, uint32_t(remaining_len));
      } else {
        *result = StringVal("");
      }
      return;
    }
    // Are we at the correct field?
    if (index == field.val_) {
      *result = StringVal(curr, uint32_t(next_delim - curr));
      return;
    }
    // We haven't reached the field yet, move along
    curr = next_delim + delim.len_;
  }
}

void StringFunctions::Repeat(exec::ExecutionContext *ctx, StringVal *result, const StringVal &str, const Integer &n) {
  if (str.is_null_ || n.is_null_) {
    *result = StringVal::Null();
    return;
  }

  if (str.len_ == 0 || n.val_ <= 0) {
    *result = StringVal("");
    return;
  }

  char *ptr = StringVal::PreAllocate(result, ctx->GetStringAllocator(), static_cast<uint32_t>(str.len_ * n.val_));
  if (UNLIKELY(ptr == nullptr)) {
    // Allocation failed
    return;
  }

  // Repeat
  auto *src = str.Content();
  for (uint32_t i = 0; i < n.val_; i++) {
    std::memcpy(ptr, src, str.len_);
    ptr += str.len_;
  }
}

void StringFunctions::Lpad(exec::ExecutionContext *ctx, StringVal *result, const StringVal &str, const Integer &len,
                           const StringVal &pad) {
  if (str.is_null_ || len.is_null_ || pad.is_null_ || len.val_ < 0) {
    *result = StringVal::Null();
    return;
  }

  // If target length equals input length, nothing to do
  if (len.val_ == str.len_) {
    *result = str;
    return;
  }

  // If target length is less than input length, truncate.
  if (len.val_ < str.len_) {
    *result = StringVal(str.Content(), uint32_t(len.val_));
    return;
  }

  char *ptr = StringVal::PreAllocate(result, ctx->GetStringAllocator(), static_cast<uint32_t>(len.val_));
  if (UNLIKELY(ptr == nullptr)) {
    // Allocation failed
    return;
  }

  auto *pad_src = pad.Content();
  for (auto bytes_left = uint32_t(len.val_ - str.len_); bytes_left > 0;) {
    auto copy_len = std::min(pad.len_, bytes_left);
    std::memcpy(ptr, pad_src, copy_len);
    bytes_left -= copy_len;
    ptr += copy_len;
  }

  std::memcpy(ptr, str.Content(), str.len_);
}

void StringFunctions::Rpad(exec::ExecutionContext *ctx, StringVal *result, const StringVal &str, const Integer &len,
                           const StringVal &pad) {
  if (str.is_null_ || len.is_null_ || pad.is_null_ || len.val_ < 0) {
    *result = StringVal::Null();
    return;
  }

  // If target length equals input length, nothing to do
  if (len.val_ == str.len_) {
    *result = str;
    return;
  }

  // If target length is less than input length, truncate.
  if (len.val_ < str.len_) {
    *result = StringVal(str.Content(), uint32_t(len.val_));
    return;
  }

  char *ptr = StringVal::PreAllocate(result, ctx->GetStringAllocator(), static_cast<uint32_t>(len.val_));
  if (UNLIKELY(ptr == nullptr)) {
    // Allocation failed
    return;
  }

  // Copy input string first
  std::memcpy(ptr, str.Content(), str.len_);
  ptr += str.len_;

  // Then padding
  auto *pad_src = pad.Content();
  for (auto bytes_left = uint32_t(len.val_ - str.len_); bytes_left > 0;) {
    auto copy_len = std::min(pad.len_, bytes_left);
    std::memcpy(ptr, pad_src, copy_len);
    bytes_left -= copy_len;
    ptr += copy_len;
  }
}

void StringFunctions::Nextval(exec::ExecutionContext *ctx, Integer *result, const StringVal &str) {
  auto accessor = ctx->GetAccessor();
  std::string_view s_v = str.StringView();
  std::string s(s_v.data(), s_v.size());

  auto txn_manager = ctx->GetTransactionManager();
  auto mini_txn_context = txn_manager->BeginTransaction();
  auto mini_txn = common::ManagedPointer<transaction::TransactionContext>(mini_txn_context);

  auto sequence_oid = accessor->GetSequenceOid(s);
  if (sequence_oid == catalog::INVALID_SEQUENCE_OID) {
    throw terrier::CATALOG_EXCEPTION("Invalid sequence");
  }

  // Using pg_sequence_index to get corresponding sequence tuple
  auto pg_sequence_index = accessor->GetIndex(catalog::postgres::SEQUENCE_OID_INDEX_OID);
  auto pg_sequence_index_pri = pg_sequence_index->GetProjectedRowInitializer();
  byte *const pg_index_buffer = common::AllocationUtil::AllocateAligned(pg_sequence_index_pri.ProjectedRowSize());
  auto pg_index_pr = pg_sequence_index_pri.InitializeRow(pg_index_buffer);
  *(reinterpret_cast<catalog::sequence_oid_t *>(pg_index_pr->AccessForceNotNull(0))) = sequence_oid;
  std::vector<storage::TupleSlot> index_scan_result;

  pg_sequence_index->ScanKey(*ctx->GetTxn().Get(), *pg_index_pr, &index_scan_result);

  TERRIER_ASSERT(index_scan_result.size() == 1, "Can't find the sequence or have duplicate sequences");

  // Using TupleAccessStrategy to get nextval value
  auto const pg_index_datatable = index_scan_result[0].GetBlock()->data_table_;
  storage::BlockLayout layout = pg_index_datatable->GetBlockLayout();
  storage::TupleAccessStrategy tuple_access_strategy(layout);

  // Write to redo log, and update in sqltable
  auto sequence_table_oid = accessor->GetTableOid("pg_sequence");
  TERRIER_ASSERT(sequence_table_oid != catalog::INVALID_TABLE_OID, "Sequence table oid should not be invalid");
  auto sequence_table = accessor->GetTable(sequence_table_oid).Get();
  auto sequence_columns = accessor->GetSchema(sequence_table_oid).GetColumns();
  const std::vector<catalog::col_oid_t> sequence_columns_oids{catalog::postgres::PG_SEQUENCE_ALL_COL_OIDS.cbegin(),
                                                              catalog::postgres::PG_SEQUENCE_ALL_COL_OIDS.cend()};
  auto sequence_pri = sequence_table->InitializerForProjectedRow(sequence_columns_oids);
  auto sequence_update_redo = mini_txn->StageWrite(ctx->DBOid(), sequence_table_oid, sequence_pri);
  auto const sequence_projection_map = sequence_table->ProjectionMapForOids(sequence_columns_oids);

  for (uint16_t i = 0; i < sequence_update_redo->Delta()->NumColumns(); i++) {
    storage::StorageUtil::CopyAttrIntoProjection(tuple_access_strategy, index_scan_result[0],
                                                 sequence_update_redo->Delta(),
                                                 sequence_projection_map.at(sequence_columns_oids[i]));
  }

  // Read sequence parameters
  auto seq_start = *reinterpret_cast<int64_t *>(tuple_access_strategy.AccessWithoutNullCheck(
      index_scan_result[0], layout.AllColumns()[sequence_projection_map.at(catalog::postgres::SEQSTART_COL_OID)]));
  auto seq_increment = *reinterpret_cast<int64_t *>(tuple_access_strategy.AccessWithoutNullCheck(
      index_scan_result[0], layout.AllColumns()[sequence_projection_map.at(catalog::postgres::SEQINCREMENT_COL_OID)]));
  auto seq_max = *reinterpret_cast<int64_t *>(tuple_access_strategy.AccessWithoutNullCheck(
      index_scan_result[0], layout.AllColumns()[sequence_projection_map.at(catalog::postgres::SEQMAX_COL_OID)]));
  auto seq_min = *reinterpret_cast<int64_t *>(tuple_access_strategy.AccessWithoutNullCheck(
      index_scan_result[0], layout.AllColumns()[sequence_projection_map.at(catalog::postgres::SEQMIN_COL_OID)]));
  auto seq_cycle = *reinterpret_cast<bool *>(tuple_access_strategy.AccessWithoutNullCheck(
      index_scan_result[0], layout.AllColumns()[sequence_projection_map.at(catalog::postgres::SEQCYCLE_COL_OID)]));

  // Read sequence value, if nullptr then set to start value and skip increment
  int64_t seq_val;
  auto seq_val_ptr = reinterpret_cast<int64_t *>(tuple_access_strategy.AccessWithNullCheck(
      index_scan_result[0], layout.AllColumns()[sequence_projection_map.at(catalog::postgres::SEQLASTVAL_COL_OID)]));
  if (seq_val_ptr == nullptr) {
    seq_val = seq_start;
  } else {
    seq_val = *seq_val_ptr;
    // Increment sequence
    if (seq_increment > 0) {
      // Ascending sequence
      if ((seq_max >= 0 && seq_val > seq_max - seq_increment) || (seq_max < 0 && seq_val + seq_increment > seq_max)) {
        if (!seq_cycle) {
          throw terrier::CATALOG_EXCEPTION("Reached maximum value of sequence");
        }
        seq_val = seq_min;
      } else {
        seq_val += seq_increment;
      }
    } else {
      // Descending sequence
      if ((seq_min < 0 && seq_val < seq_min - seq_increment) || (seq_min >= 0 && seq_val + seq_increment < seq_min)) {
        if (!seq_cycle) {
          throw terrier::CATALOG_EXCEPTION("Reached minimum value of sequence");
        }
        seq_val = seq_max;
      } else {
        seq_val += seq_increment;
      }
    }
  }

  // Update sequence value
  auto *sequence_nextval_ptr = sequence_update_redo->Delta()->AccessForceNotNull(
      sequence_projection_map.at(catalog::postgres::SEQLASTVAL_COL_OID));
  *(reinterpret_cast<int64_t *>(sequence_nextval_ptr)) = seq_val;

  sequence_update_redo->SetTupleSlot(index_scan_result[0]);
  sequence_table->Update(mini_txn, sequence_update_redo);

  // Update or insert nextval into temp_table with mini_txn
  auto temp_table_oid = ctx->GetTempTable();
  auto temp_table = accessor->GetTable(temp_table_oid).Get();
  auto temp_columns = accessor->GetSchema(temp_table_oid).GetColumns();

  // Sequence table only have two colums right now
  const std::vector<catalog::col_oid_t> temp_colums_oids{temp_columns[0].Oid(), temp_columns[1].Oid()};
  auto temp_pri = temp_table->InitializerForProjectedRow(temp_colums_oids);
  auto *const temp_insert_redo = mini_txn->StageWrite(ctx->DBOid(), temp_table_oid, temp_pri);
  auto *const temp_insert_pr = temp_insert_redo->Delta();
  auto const temp_projection_map = temp_table->ProjectionMapForOids(temp_colums_oids);
  // First column is session id
  auto *first_col_oid_ptr = temp_insert_pr->AccessForceNotNull(temp_projection_map.at(temp_colums_oids[0]));
  *(reinterpret_cast<catalog::sequence_oid_t *>(first_col_oid_ptr)) = sequence_oid;
  // Second colum is last next val
  auto *second_col_oid_ptr = temp_insert_pr->AccessForceNotNull(temp_projection_map.at(temp_colums_oids[1]));
  *(reinterpret_cast<int64_t *>(second_col_oid_ptr)) = seq_val;

  result->is_null_ = str.is_null_;
  result->val_ = seq_val;

  // Initiate projected columns to hold scan result
  const auto pci = temp_table->InitializerForProjectedColumns(temp_colums_oids, 1);
  byte *buffer = common::AllocationUtil::AllocateAligned(pci.ProjectedColumnsSize());
  auto pc = pci.Initialize(buffer);

  auto seq_oid_ptrs =
      reinterpret_cast<catalog::sequence_oid_t *>(pc->ColumnStart(temp_projection_map.at(temp_colums_oids[0])));

  std::vector<catalog::sequence_oid_t> db_cats;
  auto table_iter = temp_table->begin();
  auto prev_table_iter = table_iter;

  // Scan one row at a time into pc
  while (table_iter != temp_table->end()) {
    temp_table->Scan(mini_txn, &table_iter, pc);
    // Has sequence entry, update instead of insert
    if (seq_oid_ptrs[0] == sequence_oid) {
      temp_insert_redo->SetTupleSlot(*prev_table_iter);
      temp_table->Update(mini_txn, temp_insert_redo);
      txn_manager->Commit(mini_txn_context, transaction::TransactionUtil::EmptyCallback, nullptr);
      return;
    }
    prev_table_iter = table_iter;
  }

  temp_table->Insert(mini_txn, temp_insert_redo);
  txn_manager->Commit(mini_txn_context, transaction::TransactionUtil::EmptyCallback, nullptr);
}

void StringFunctions::Currval(exec::ExecutionContext *ctx, Integer *result, const StringVal &str) {
  auto accessor = ctx->GetAccessor();
  std::string_view s_v = str.StringView();
  std::string s(s_v.data(), s_v.size());

  auto sequence_oid = accessor->GetSequenceOid(s);
  auto temp_table_oid = ctx->GetTempTable();
  auto temp_table = accessor->GetTable(temp_table_oid).Get();
  auto temp_colums = accessor->GetSchema(temp_table_oid).GetColumns();

  auto txn_manager = ctx->GetTransactionManager();
  auto mini_txn_context = txn_manager->BeginTransaction();
  auto mini_txn = common::ManagedPointer<transaction::TransactionContext>(mini_txn_context);

  // Sequence table only have two colums right now
  const std::vector<catalog::col_oid_t> temp_colums_oids{temp_colums[0].Oid(), temp_colums[1].Oid()};
  auto const table_projection_map = temp_table->ProjectionMapForOids(temp_colums_oids);

  // Initiate projected columns to hold scan result
  const auto pci = temp_table->InitializerForProjectedColumns(temp_colums_oids, 100);
  byte *buffer = common::AllocationUtil::AllocateAligned(pci.ProjectedColumnsSize());
  auto pc = pci.Initialize(buffer);

  auto seq_oid_ptrs =
      reinterpret_cast<catalog::sequence_oid_t *>(pc->ColumnStart(table_projection_map.at(temp_colums_oids[0])));
  auto seq_value_ptrs = reinterpret_cast<int64_t *>(pc->ColumnStart(table_projection_map.at(temp_colums_oids[1])));

  std::vector<catalog::sequence_oid_t> db_cats;
  auto table_iter = temp_table->begin();

  // Scan 100 at a time into pc
  while (table_iter != temp_table->end()) {
    temp_table->Scan(mini_txn, &table_iter, pc);

    for (uint i = 0; i < pc->NumTuples(); i++) {
      if (seq_oid_ptrs[i] == sequence_oid) {
        result->val_ = seq_value_ptrs[i];
        result->is_null_ = str.is_null_;
        txn_manager->Commit(mini_txn_context, transaction::TransactionUtil::EmptyCallback, nullptr);
        return;
      }
    }
  }
  txn_manager->Commit(mini_txn_context, transaction::TransactionUtil::EmptyCallback, nullptr);
  throw terrier::CATALOG_EXCEPTION("Nextval has never been called in this session");
}

void StringFunctions::Length(UNUSED_ATTRIBUTE exec::ExecutionContext *ctx, Integer *result, const StringVal &str) {
  result->is_null_ = str.is_null_;
  result->val_ = str.len_;
}

void StringFunctions::Lower(exec::ExecutionContext *ctx, StringVal *result, const StringVal &str) {
  if (str.is_null_) {
    *result = StringVal::Null();
    return;
  }

  char *ptr = StringVal::PreAllocate(result, ctx->GetStringAllocator(), str.len_);
  if (UNLIKELY(ptr == nullptr)) {
    // Allocation failed
    return;
  }

  auto *src = str.Content();
  for (uint32_t i = 0; i < str.len_; i++) {
    ptr[i] = static_cast<char>(std::tolower(src[i]));
  }
}

void StringFunctions::Upper(exec::ExecutionContext *ctx, StringVal *result, const StringVal &str) {
  if (str.is_null_) {
    *result = StringVal::Null();
    return;
  }

  char *ptr = StringVal::PreAllocate(result, ctx->GetStringAllocator(), str.len_);
  if (UNLIKELY(ptr == nullptr)) {
    // Allocation failed
    return;
  }

  auto *src = str.Content();
  for (uint32_t i = 0; i < str.len_; i++) {
    ptr[i] = static_cast<char>(std::toupper(src[i]));
  }
}

void StringFunctions::Reverse(exec::ExecutionContext *ctx, StringVal *result, const StringVal &str) {
  if (str.is_null_) {
    *result = StringVal::Null();
    return;
  }

  if (str.len_ == 0) {
    *result = str;
    return;
  }

  char *ptr = StringVal::PreAllocate(result, ctx->GetStringAllocator(), str.len_);
  if (UNLIKELY(ptr == nullptr)) {
    // Allocation failed
    return;
  }

  auto *src = str.Content();
  std::reverse_copy(src, src + str.len_, ptr);
}

namespace {

// TODO(pmenon): The bitset we use can be prepared once before all function
//               invocations if the characters list is a constant value, and not
//               a column value (i.e., SELECT ltrim(col, 'abc') FROM ...). This
//               should be populated in the execution context once apriori
//               rather that initializing it each invocation.
// TODO(pmenon): What about non-ASCII strings?
// Templatized from Postgres
template <bool TrimLeft, bool TrimRight>
void DoTrim(StringVal *result, const StringVal &str, const StringVal &chars) {
  if (str.is_null_ || chars.is_null_) {
    *result = StringVal::Null();
    return;
  }

  if (str.len_ == 0) {
    *result = str;
    return;
  }

  util::InlinedBitVector<256> bitset;
  // Store this variable to avoid reexecuting if statements.
  auto *chars_content = chars.Content();
  for (uint32_t i = 0; i < chars.len_; i++) {
    bitset.Set(uint32_t(chars_content[i]));
  }

  // The valid range
  int32_t begin = 0, end = str.len_ - 1;

  auto *src = str.Content();
  // NOLINTNEXTLINE: bugprone-suspicious-semicolon: seems like a false positive because of constexpr
  if constexpr (TrimLeft) {
    while (begin < static_cast<int32_t>(str.len_) && bitset.Test(uint32_t(src[begin]))) {
      begin++;
    }
  }

  // NOLINTNEXTLINE: bugprone-suspicious-semicolon: seems like a false positive because of constexpr
  if constexpr (TrimRight) {
    while (begin <= end && bitset.Test(uint32_t(src[end]))) {
      end--;
    }
  }

  *result = StringVal(src + begin, uint32_t(end - begin + 1));
}

}  // namespace

void StringFunctions::Trim(UNUSED_ATTRIBUTE exec::ExecutionContext *ctx, StringVal *result, const StringVal &str) {
  DoTrim<true, true>(result, str, StringVal(" "));
}

void StringFunctions::Trim(UNUSED_ATTRIBUTE exec::ExecutionContext *ctx, StringVal *result, const StringVal &str,
                           const StringVal &chars) {
  DoTrim<true, true>(result, str, chars);
}

void StringFunctions::Ltrim(UNUSED_ATTRIBUTE exec::ExecutionContext *ctx, StringVal *result, const StringVal &str) {
  DoTrim<true, false>(result, str, StringVal(" "));
}

void StringFunctions::Ltrim(UNUSED_ATTRIBUTE exec::ExecutionContext *ctx, StringVal *result, const StringVal &str,
                            const StringVal &chars) {
  DoTrim<true, false>(result, str, chars);
}

void StringFunctions::Rtrim(UNUSED_ATTRIBUTE exec::ExecutionContext *ctx, StringVal *result, const StringVal &str) {
  DoTrim<false, true>(result, str, StringVal(" "));
}

void StringFunctions::Rtrim(UNUSED_ATTRIBUTE exec::ExecutionContext *ctx, StringVal *result, const StringVal &str,
                            const StringVal &chars) {
  DoTrim<false, true>(result, str, chars);
}

void StringFunctions::Left(UNUSED_ATTRIBUTE exec::ExecutionContext *ctx, StringVal *result, const StringVal &str,
                           const Integer &n) {
  if (str.is_null_ || n.is_null_) {
    *result = StringVal::Null();
    return;
  }

  const auto len =
      n.val_ < 0 ? std::max(int64_t{0}, str.len_ + n.val_) : std::min(str.len_, static_cast<uint32_t>(n.val_));
  *result = StringVal(str.Content(), uint32_t(len));
}

void StringFunctions::Right(UNUSED_ATTRIBUTE exec::ExecutionContext *ctx, StringVal *result, const StringVal &str,
                            const Integer &n) {
  if (str.is_null_ || n.is_null_) {
    *result = StringVal::Null();
    return;
  }

  const auto len = std::min(str.len_, static_cast<uint32_t>(std::abs(n.val_)));
  if (n.val_ > 0) {
    *result = StringVal(str.Content() + (str.len_ - len), len);
  } else {
    *result = StringVal(str.Content() + len, str.len_ - len);
  }
}

}  // namespace terrier::execution::sql
