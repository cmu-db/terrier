#pragma once
#include <functional>
#include "fake_result_set.h"
#include "network/postgres_protocol_utils.h"

struct sqlite3;

namespace terrier::traffic_cop {

class SqliteEngine
{

 public:
  SqliteEngine();
  virtual ~SqliteEngine();

  void ExecuteQuery(const char *query,
                      network::PostgresPacketWriter *out,
                      std::function<void(FakeResultSet & , network::PostgresPacketWriter * )> &callback);

 private:
  // SQLite database
  struct sqlite3 *sqlite_db_;
  char *error_msg;

  static int StoreResults(void* result_set, int elem_count, char** values, char** column_names);

};

} // namespace terrier::traffic_cop

