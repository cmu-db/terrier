#pragma once

#include "self_driving/pilot/action/abstract_action.h"

namespace noisepage::selfdriving::pilot {

/**
 * Represent a drop index self-driving action
 */
class DropIndexAction : public AbstractAction {
 public:
  /**
   * Construct CreateIndexAction
   * @param index_name The name of the index
   * @param table_name The table to create index on
   * @param columns The columns to build index on
   */
  DropIndexAction(std::string index_name, std::string table_name, std::vector<IndexColumn> columns)
      : AbstractAction(ActionFamily::CHANGE_INDEX),
        index_name_(std::move(index_name)),
        table_name_(std::move(table_name)),
        columns_(std::move(columns)) {}

  /**
   * @return Name of the index
   */
  const std::string &GetIndexName() const { return index_name_; }

 private:
  std::string table_name_;
  std::string index_name_;
  std::vector<IndexColumn> columns_;
};

}  // namespace noisepage::selfdriving::pilot
