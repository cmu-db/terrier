#pragma once

#include <memory>

#include "common/managed_pointer.h"
#include "network/postgres/statement.h"
#include "parser/postgresparser.h"
#include "traffic_cop/traffic_cop_util.h"

namespace terrier::network {

class Statement {
 public:
  explicit Statement(std::unique_ptr<parser::ParseResult> &&parse_result)
      : parse_result_(std::move(parse_result)) {
    if (!Invalid()) {
      TERRIER_ASSERT(parse_result_->GetStatements().size() <= 1,
                     "We currently expect one statement per string (psql and oltpbench).");
      if (!Empty()) {
        root_statement_ = parse_result_->GetStatement(0);
        type_ = trafficcop::TrafficCopUtil::QueryTypeForStatement(root_statement_);
      }
    }
  }

  bool Invalid() const { return parse_result_ == nullptr; }

  bool Empty() const {
    TERRIER_ASSERT(!Invalid(), "Attempting to check emptiness without a valid parsed result.");
    return parse_result_->Empty();
  }

  common::ManagedPointer<parser::ParseResult> ParseResult() const {
    TERRIER_ASSERT(!Invalid(), "Attempting to get parse results without a valid parsed result.");
    return common::ManagedPointer(parse_result_);
  }

  common::ManagedPointer<parser::SQLStatement> RootStatement() const {
    TERRIER_ASSERT(!Invalid(), "Attempting to get root statement without a valid parsed result.");
    return common::ManagedPointer(root_statement_);
  }

  QueryType QueryType() const { return type_; }

 private:
  const std::unique_ptr<parser::ParseResult> parse_result_ = nullptr;
  common::ManagedPointer<parser::SQLStatement> root_statement_ = nullptr;
  enum QueryType type_ = QueryType::QUERY_INVALID;
};  // namespace terrier::network

}  // namespace terrier::network