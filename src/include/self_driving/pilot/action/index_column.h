#pragma once

namespace noisepage::selfdriving::pilot {

/**
 * Store the column information for the CreateIndex (and DropIndex for bookkeeping) action
 * For now only need the column name, but we may augment other information in the future
 */
struct IndexColumn {
  /**
   * Construct an IndexColumn
   * @param column_name name of the column
   */
  explicit IndexColumn(std::string column_name) : column_name_(column_name) {}

  const std::string &GetColumnName() const { return column_name_; }

  std::string column_name_;  // Column name to build index with
};

}  // namespace noisepage::selfdriving::pilot
