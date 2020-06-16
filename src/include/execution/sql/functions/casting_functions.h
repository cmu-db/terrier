#pragma once

#include <string>

#include "common/all_static.h"
#include "execution/sql/operators/cast_operators.h"
#include "execution/sql/value.h"

namespace terrier::execution::exec {
class ExecutionContext;
}

namespace terrier::execution::sql {

/**
 * Utility class to handle various SQL casting functions.
 */
class CastingFunctions : public common::AllStatic {
 public:
  static void CastToBoolVal(BoolVal *result, const Integer &v);
  static void CastToBoolVal(BoolVal *result, const Real &v);
  static void CastToBoolVal(BoolVal *result, const StringVal &v);

  static void CastToInteger(Integer *result, const BoolVal &v);
  static void CastToInteger(Integer *result, const Real &v);
  static void CastToInteger(Integer *result, const StringVal &v);

  static void CastToReal(Real *result, const BoolVal &v);
  static void CastToReal(Real *result, const Integer &v);
  static void CastToReal(Real *result, const StringVal &v);

  static void CastToDateVal(DateVal *result, const TimestampVal &v);
  static void CastToDateVal(DateVal *result, const StringVal &v);

  static void CastToTimestampVal(TimestampVal *result, const DateVal &v);
  static void CastToTimestampVal(TimestampVal *result, const StringVal &v);

  static void CastToStringVal(StringVal *result, exec::ExecutionContext *ctx, const BoolVal &v);
  static void CastToStringVal(StringVal *result, exec::ExecutionContext *ctx, const Integer &v);
  static void CastToStringVal(StringVal *result, exec::ExecutionContext *ctx, const Real &v);
  static void CastToStringVal(StringVal *result, exec::ExecutionContext *ctx, const DateVal &v);
  static void CastToStringVal(StringVal *result, exec::ExecutionContext *ctx, const TimestampVal &v);
};

// ---------------------------------------------------------
// Implementation below
// ---------------------------------------------------------

// The functions below are inlined in the header for performance. Don't move it unless you know what
// you're doing.

// TODO(pmenon): Catch cast exceptions!

#define CAST_HIDE_NULL_FAST(FROM_TYPE, TO_TYPE)                                        \
  inline void CastingFunctions::CastTo##TO_TYPE(TO_TYPE *result, const FROM_TYPE &v) { \
    using InputType = decltype(FROM_TYPE::val_);                                       \
    using OutputType = decltype(TO_TYPE::val_);                                        \
    result->is_null_ = v.is_null_;                                                     \
    terrier::execution::sql::TryCast<InputType, OutputType>{}(v.val_, &result->val_);  \
  }

#define CAST_HIDE_NULL(FROM_TYPE, TO_TYPE)                                             \
  inline void CastingFunctions::CastTo##TO_TYPE(TO_TYPE *result, const FROM_TYPE &v) { \
    using InputType = decltype(FROM_TYPE::val_);                                       \
    using OutputType = decltype(TO_TYPE::val_);                                        \
    if (v.is_null_) {                                                                  \
      *result = TO_TYPE::Null();                                                       \
      return;                                                                          \
    }                                                                                  \
    OutputType output{};                                                               \
    terrier::execution::sql::TryCast<InputType, OutputType>{}(v.val_, &output);        \
    *result = TO_TYPE(output);                                                         \
  }

// Something to boolean.
CAST_HIDE_NULL_FAST(Integer, BoolVal);
CAST_HIDE_NULL_FAST(Real, BoolVal);
CAST_HIDE_NULL(StringVal, BoolVal);
// Something to integer.
CAST_HIDE_NULL_FAST(BoolVal, Integer);
CAST_HIDE_NULL_FAST(Real, Integer);
CAST_HIDE_NULL(StringVal, Integer);
// Something to real.
CAST_HIDE_NULL_FAST(BoolVal, Real);
CAST_HIDE_NULL_FAST(Integer, Real);
CAST_HIDE_NULL(StringVal, Real);
// Something to date.
CAST_HIDE_NULL_FAST(TimestampVal, DateVal);
CAST_HIDE_NULL(StringVal, DateVal);
// Something to timestamp.
CAST_HIDE_NULL_FAST(DateVal, TimestampVal);
CAST_HIDE_NULL(StringVal, TimestampVal);

#undef CAST_HIDE_NULL
#undef CAST_HIDE_NULL_FAST

// Something to string.
#define CAST_TO_STRING(FROM_TYPE)                                                                     \
  inline void CastingFunctions::CastToStringVal(StringVal *result, exec::ExecutionContext *const ctx, \
                                                const FROM_TYPE &v) {                                 \
    /* TODO(pmenon): Perform an explicit if-check here because we expect string */                    \
    /*               parsing to be costlier than a branch mis-prediction. Check! */                   \
    if (v.is_null_) {                                                                                 \
      *result = StringVal::Null();                                                                    \
      return;                                                                                         \
    }                                                                                                 \
    const auto str = terrier::execution::sql::Cast<decltype(FROM_TYPE::val_), std::string>{}(v.val_); \
    *result = StringVal(ctx->GetStringAllocator()->AddVarlen(str));                                   \
  }

CAST_TO_STRING(BoolVal);
CAST_TO_STRING(Integer);
CAST_TO_STRING(Real);
CAST_TO_STRING(DateVal);
CAST_TO_STRING(TimestampVal);

#undef CAST_TO_STRING

}  // namespace terrier::execution::sql
