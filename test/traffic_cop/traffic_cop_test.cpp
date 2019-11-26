#include <pqxx/pqxx> /* libpqxx is used to instantiate C++ client */

#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

#include "common/settings.h"
#include "gtest/gtest.h"
#include "loggers/main_logger.h"
#include "network/connection_handle_factory.h"
#include "network/terrier_server.h"
#include "storage/garbage_collector.h"
#include "test_util/manual_packet_util.h"
#include "test_util/test_harness.h"
#include "traffic_cop/traffic_cop.h"
#include "traffic_cop/traffic_cop_defs.h"
#include "transaction/deferred_action_manager.h"
#include "transaction/transaction_manager.h"

namespace terrier {
class TrafficCopTests : public TerrierTest {
 protected:
  std::unique_ptr<network::TerrierServer> server_;
  uint16_t port_ = common::Settings::SERVER_PORT;
  std::thread server_thread_;

  common::DedicatedThreadRegistry thread_registry_{DISABLED};
  storage::RecordBufferSegmentPool buffer_segment_pool_{100000, 1000};
  transaction::TimestampManager timestamp_manager_{};
  transaction::DeferredActionManager deferred_action_manager_{&timestamp_manager_};
  transaction::TransactionManager txn_manager_{&timestamp_manager_, &deferred_action_manager_, &buffer_segment_pool_,
                                               true, DISABLED};
  storage::BlockStore block_store_{10000, 10000};
  catalog::Catalog catalog_{&txn_manager_, &block_store_};
  storage::GarbageCollector gc_{&timestamp_manager_, &deferred_action_manager_, &txn_manager_, DISABLED};

  std::unique_ptr<trafficcop::TrafficCop> tcop_;

  network::PostgresCommandFactory command_factory_;
  network::PostgresProtocolInterpreter::Provider interpreter_provider_{common::ManagedPointer(&command_factory_)};
  std::unique_ptr<network::ConnectionHandleFactory> handle_factory_;

  void SetUp() override {
    TerrierTest::SetUp();

    auto bootstrap_txn = txn_manager_.BeginTransaction();
    catalog_.CreateDatabase(bootstrap_txn, catalog::DEFAULT_DATABASE, true);
    txn_manager_.Commit(bootstrap_txn, transaction::TransactionUtil::EmptyCallback, nullptr);

    gc_.PerformGarbageCollection();
    gc_.PerformGarbageCollection();

    tcop_ = std::make_unique<trafficcop::TrafficCop>(common::ManagedPointer(&txn_manager_),
                                                     common::ManagedPointer(&catalog_));

    network::network_logger->set_level(spdlog::level::trace);
    test_logger->set_level(spdlog::level::debug);
    spdlog::flush_every(std::chrono::seconds(1));

    try {
      handle_factory_ = std::make_unique<network::ConnectionHandleFactory>(common::ManagedPointer(tcop_));
      server_ = std::make_unique<network::TerrierServer>(
          common::ManagedPointer<network::ProtocolInterpreter::Provider>(&interpreter_provider_),
          common::ManagedPointer(handle_factory_.get()),
          common::ManagedPointer<common::DedicatedThreadRegistry>(&thread_registry_));
      server_->SetPort(port_);
      server_->RunServer();
    } catch (NetworkProcessException &exception) {
      TEST_LOG_ERROR("[LaunchServer] exception when launching server");
      throw;
    }
    TEST_LOG_DEBUG("Server initialized");
  }

  void TearDown() override {
    server_->StopServer();
    TEST_LOG_DEBUG("Terrier has shut down");
    catalog_.TearDown();
    gc_.PerformGarbageCollection();
    gc_.PerformGarbageCollection();
    gc_.PerformGarbageCollection();
    TerrierTest::TearDown();
  }

  // The port used to connect a Postgres backend. Useful for debugging.
  const int postgres_port_ = 5432;
};

// NOLINTNEXTLINE
TEST_F(TrafficCopTests, RoundTripTest) {
  try {
    pqxx::connection connection(fmt::format("host=127.0.0.1 port={0} user={1} sslmode=disable application_name=psql",
                                            port_, catalog::DEFAULT_DATABASE));

    pqxx::work txn1(connection);
    txn1.exec("DROP TABLE IF EXISTS TableA");
    txn1.exec("CREATE TABLE TableA (id INT PRIMARY KEY, data TEXT);");
    txn1.exec("INSERT INTO TableA VALUES (1, 'abc');");

    pqxx::result r = txn1.exec("SELECT * FROM TableA");
    for (const pqxx::row &row : r) {
      std::string row_str;
      for (const pqxx::field &col : row) {
        row_str += col.c_str();
        row_str += '\t';
      }
      TEST_LOG_INFO(row_str);
    }
    EXPECT_EQ(r.size(), 1);
    txn1.commit();
    connection.disconnect();
  } catch (const std::exception &e) {
    TEST_LOG_ERROR("Exception occurred: {0}", e.what());
    EXPECT_TRUE(false);
  }
}

/**
 * Test whether a temporary namespace is created for a connection to the database
 */
// NOLINTNEXTLINE
TEST_F(TrafficCopTests, TemporaryNamespaceTest) {
  try {
    std::thread([this] {
      pqxx::connection connection(fmt::format("host=127.0.0.1 port={0} user={1} sslmode=disable application_name=psql",
                                              port_, catalog::DEFAULT_DATABASE));

      pqxx::work txn1(connection);

      auto txn = txn_manager_.BeginTransaction();
      auto db_accessor = catalog_.GetAccessor(txn, catalog_.GetDatabaseOid(txn, catalog::DEFAULT_DATABASE));

      // Create a new namespace and make sure that its OID is higher than the default start OID,
      // which should have been assigned to the temporary namespace for this connection
      auto new_namespace_oid = db_accessor->CreateNamespace(std::string(trafficcop::TEMP_NAMESPACE_PREFIX));
      EXPECT_GT(static_cast<uint32_t>(new_namespace_oid), catalog::START_OID);
      txn_manager_.Commit(txn, transaction::TransactionUtil::EmptyCallback, nullptr);
      txn1.commit();
      connection.disconnect();
    });
  } catch (const std::exception &e) {
    TEST_LOG_ERROR("Exception occurred: {0}", e.what());
    EXPECT_TRUE(false);
  }
}

// NOLINTNEXTLINE
TEST_F(TrafficCopTests, ManualExtendedQueryTest) {
  try {
    auto io_socket_unique_ptr = network::ManualPacketUtil::StartConnection(port_);
    auto io_socket = common::ManagedPointer(io_socket_unique_ptr);
    network::PostgresPacketWriter writer(io_socket->GetWriteQueue());

    writer.WriteSimpleQuery("DROP TABLE IF EXISTS TableA");
    io_socket->FlushAllWrites();
    network::ManualPacketUtil::ReadUntilReadyOrClose(io_socket);

    writer.WriteSimpleQuery(
        "CREATE TABLE TableA (a_int INT PRIMARY KEY, a_dec DECIMAL, a_text TEXT, a_time TIMESTAMP, a_bigint BIGINT);");
    io_socket->FlushAllWrites();
    network::ManualPacketUtil::ReadUntilReadyOrClose(io_socket);

    writer.WriteSimpleQuery("INSERT INTO TableA VALUES(100, 3.14, 'nico', 114514, 1234)");
    io_socket->FlushAllWrites();
    network::ManualPacketUtil::ReadUntilReadyOrClose(io_socket);

    std::string stmt_name = "begin_statement";
    std::string query = "BEGIN";

    writer.WriteParseCommand(stmt_name, query, std::vector<int>());
    io_socket->FlushAllWrites();
    network::ManualPacketUtil::ReadUntilMessageOrClose(io_socket, network::NetworkMessageType::PG_PARSE_COMPLETE);

    {
      std::string portal_name = "test_portal";
      // Use text format, don't care about result column formats
      writer.WriteBindCommand(portal_name, stmt_name, {}, {}, {});
      io_socket->FlushAllWrites();
      network::ManualPacketUtil::ReadUntilMessageOrClose(io_socket, network::NetworkMessageType::PG_BIND_COMPLETE);

      writer.WriteExecuteCommand(portal_name, 0);
      io_socket->FlushAllWrites();
      network::ManualPacketUtil::ReadUntilMessageOrClose(io_socket, network::NetworkMessageType::PG_COMMAND_COMPLETE);

      writer.WriteSyncCommand();
      io_socket->FlushAllWrites();
      network::ManualPacketUtil::ReadUntilReadyOrClose(io_socket);
    }

    stmt_name = "test_statement";
    query = "SELECT * FROM TableA WHERE a_int = $1 AND a_dec = $2 AND a_text = $3 AND a_time = $4 AND a_bigint = $5";

    writer.WriteParseCommand(stmt_name, query,
                             std::vector<int>({static_cast<int32_t>(network::PostgresValueType::INTEGER),
                                               static_cast<int32_t>(network::PostgresValueType::DECIMAL),
                                               static_cast<int32_t>(network::PostgresValueType::VARCHAR),
                                               static_cast<int32_t>(network::PostgresValueType::TIMESTAMPS),
                                               static_cast<int32_t>(network::PostgresValueType::BIGINT)}));
    io_socket->FlushAllWrites();
    network::ManualPacketUtil::ReadUntilMessageOrClose(io_socket, network::NetworkMessageType::PG_PARSE_COMPLETE);

    // Bind, param = "100", "3.14", "nico", "114514" expressed in vector form
    auto param1 = std::vector<char>({'1', '0', '0'});
    auto param2 = std::vector<char>({'3', '.', '1', '4'});
    auto param3 = std::vector<char>({'n', 'i', 'c', 'o'});
    auto param4 = std::vector<char>({'1', '1', '4', '5', '1', '4'});
    auto param5 = std::vector<char>({'1', '2', '3', '4'});

    {
      std::string portal_name = "test_portal";
      // Use text format, don't care about result column formats
      writer.WriteBindCommand(portal_name, stmt_name, {}, {&param1, &param2, &param3, &param4, &param5}, {});
      io_socket->FlushAllWrites();
      network::ManualPacketUtil::ReadUntilMessageOrClose(io_socket, network::NetworkMessageType::PG_BIND_COMPLETE);

      writer.WriteDescribeCommand(network::DescribeCommandObjectType::STATEMENT, stmt_name);
      io_socket->FlushAllWrites();
      network::ManualPacketUtil::ReadUntilMessageOrClose(io_socket,
                                                         network::NetworkMessageType::PG_PARAMETER_DESCRIPTION);

      writer.WriteDescribeCommand(network::DescribeCommandObjectType::PORTAL, portal_name);
      io_socket->FlushAllWrites();
      network::ManualPacketUtil::ReadUntilMessageOrClose(io_socket, network::NetworkMessageType::PG_ROW_DESCRIPTION);

      writer.WriteExecuteCommand(portal_name, 0);
      io_socket->FlushAllWrites();
      network::ManualPacketUtil::ReadUntilMessageOrClose(io_socket, network::NetworkMessageType::PG_DATA_ROW);

      writer.WriteSyncCommand();
      io_socket->FlushAllWrites();
      network::ManualPacketUtil::ReadUntilReadyOrClose(io_socket);
    }

    {
      // Test single specifier for all paramters (1)
      // TODO(Weichen): Test binary format here

      std::string portal_name = "test_portal-2";
      // Use text format, don't care about result column formats, specify "0" for using text for all params
      writer.WriteBindCommand(portal_name, stmt_name, {0}, {&param1, &param2, &param3, &param4, &param5}, {});
      io_socket->FlushAllWrites();
      network::ManualPacketUtil::ReadUntilMessageOrClose(io_socket, network::NetworkMessageType::PG_BIND_COMPLETE);

      writer.WriteDescribeCommand(network::DescribeCommandObjectType::PORTAL, portal_name);
      io_socket->FlushAllWrites();
      network::ManualPacketUtil::ReadUntilMessageOrClose(io_socket, network::NetworkMessageType::PG_ROW_DESCRIPTION);

      writer.WriteExecuteCommand(portal_name, 0);
      io_socket->FlushAllWrites();
      network::ManualPacketUtil::ReadUntilMessageOrClose(io_socket, network::NetworkMessageType::PG_DATA_ROW);

      writer.WriteSyncCommand();
      io_socket->FlushAllWrites();
      network::ManualPacketUtil::ReadUntilReadyOrClose(io_socket);
    }

    {
      // Test individual specifier for each parameter

      std::string portal_name = "test_portal-3";
      // Use text format, don't care about result column formats
      writer.WriteBindCommand(portal_name, stmt_name, {0, 0, 0, 0, 0}, {&param1, &param2, &param3, &param4, &param5},
                              {});
      io_socket->FlushAllWrites();
      network::ManualPacketUtil::ReadUntilMessageOrClose(io_socket, network::NetworkMessageType::PG_BIND_COMPLETE);

      writer.WriteDescribeCommand(network::DescribeCommandObjectType::PORTAL, portal_name);
      io_socket->FlushAllWrites();
      network::ManualPacketUtil::ReadUntilMessageOrClose(io_socket, network::NetworkMessageType::PG_ROW_DESCRIPTION);

      writer.WriteExecuteCommand(portal_name, 0);
      io_socket->FlushAllWrites();
      network::ManualPacketUtil::ReadUntilMessageOrClose(io_socket, network::NetworkMessageType::PG_DATA_ROW);

      writer.WriteSyncCommand();
      io_socket->FlushAllWrites();
      network::ManualPacketUtil::ReadUntilReadyOrClose(io_socket);
    }

    // CloseCommand
    writer.WriteCloseCommand(network::DescribeCommandObjectType::STATEMENT, stmt_name);
    io_socket->FlushAllWrites();

    stmt_name = "commit_statement";
    query = "COMMIT";

    writer.WriteParseCommand(stmt_name, query, std::vector<int>());
    io_socket->FlushAllWrites();
    network::ManualPacketUtil::ReadUntilMessageOrClose(io_socket, network::NetworkMessageType::PG_PARSE_COMPLETE);

    {
      std::string portal_name = "test_portal";
      // Use text format, don't care about result column formats
      writer.WriteBindCommand(portal_name, stmt_name, {}, {}, {});
      io_socket->FlushAllWrites();
      network::ManualPacketUtil::ReadUntilMessageOrClose(io_socket, network::NetworkMessageType::PG_BIND_COMPLETE);

      writer.WriteExecuteCommand(portal_name, 0);
      io_socket->FlushAllWrites();

      writer.WriteSyncCommand();
      io_socket->FlushAllWrites();
      network::ManualPacketUtil::ReadUntilReadyOrClose(io_socket);
    }

    network::ManualPacketUtil::TerminateConnection(io_socket->GetSocketFd());
    io_socket->Close();
  } catch (const std::exception &e) {
    TEST_LOG_ERROR("Exception occurred: {0}", e.what());
    EXPECT_TRUE(false);
  }
}

// -------------------------------------------------------------------------

/**
 * The manual tests below are for debugging. They can be disabled when testing other components.
 * You can launch a Postgres backend and compare packets from terrier and from Postgres
 * to find if you have created correct packets.
 *
 */

// NOLINTNEXTLINE
TEST_F(TrafficCopTests, ManualRoundTripTest) {
  try {
    auto io_socket_unique_ptr = network::ManualPacketUtil::StartConnection(port_);
    auto io_socket = common::ManagedPointer(io_socket_unique_ptr);
    network::PostgresPacketWriter writer(io_socket->GetWriteQueue());

    writer.WriteSimpleQuery("DROP TABLE IF EXISTS TableA");
    io_socket->FlushAllWrites();
    network::ManualPacketUtil::ReadUntilReadyOrClose(io_socket);
    writer.WriteSimpleQuery("CREATE TABLE TableA (id INT PRIMARY KEY, data TEXT);");
    io_socket->FlushAllWrites();
    network::ManualPacketUtil::ReadUntilReadyOrClose(io_socket);
    writer.WriteSimpleQuery("INSERT INTO TableA VALUES (1, 'abc');");
    io_socket->FlushAllWrites();
    network::ManualPacketUtil::ReadUntilReadyOrClose(io_socket);

    writer.WriteSimpleQuery("SELECT * FROM TableA");
    io_socket->FlushAllWrites();
    network::ManualPacketUtil::ReadUntilReadyOrClose(io_socket);
    network::ManualPacketUtil::TerminateConnection(io_socket->GetSocketFd());
    io_socket->Close();
  } catch (const std::exception &e) {
    TEST_LOG_ERROR("Exception occurred: {0}", e.what());
    EXPECT_TRUE(false);
  }
}

// NOLINTNEXTLINE
TEST_F(TrafficCopTests, ErrorHandlingTest) {
  auto io_socket_unique_ptr = network::ManualPacketUtil::StartConnection(port_);
  auto io_socket = common::ManagedPointer(io_socket_unique_ptr);
  network::PostgresPacketWriter writer(io_socket->GetWriteQueue());

  writer.WriteSimpleQuery("DROP TABLE IF EXISTS TableA");
  io_socket->FlushAllWrites();
  network::ManualPacketUtil::ReadUntilReadyOrClose(io_socket);
  writer.WriteSimpleQuery("CREATE TABLE TableA (id INT PRIMARY KEY, data TEXT);");
  io_socket->FlushAllWrites();
  network::ManualPacketUtil::ReadUntilReadyOrClose(io_socket);
  writer.WriteSimpleQuery("INSERT INTO TableA VALUES (1, 'abc');");
  io_socket->FlushAllWrites();
  network::ManualPacketUtil::ReadUntilReadyOrClose(io_socket);

  std::string stmt_name = "test_statement";
  std::string query = "SELECT * FROM TableA WHERE id = $1";
  writer.WriteParseCommand(stmt_name, query,
                           std::vector<int>({static_cast<int32_t>(network::PostgresValueType::INTEGER)}));
  io_socket->FlushAllWrites();
  network::ManualPacketUtil::ReadUntilMessageOrClose(io_socket, network::NetworkMessageType::PG_PARSE_COMPLETE);

  {
    // Repeated statement name
    writer.WriteParseCommand(stmt_name, query,
                             std::vector<int>({static_cast<int32_t>(network::PostgresValueType::INTEGER)}));
    io_socket->FlushAllWrites();
    network::ManualPacketUtil::ReadUntilMessageOrClose(io_socket, network::NetworkMessageType::PG_ERROR_RESPONSE);
  }

  std::string portal_name = "test_portal";
  auto param1 = std::vector<char>({'1', '0', '0'});

  {
    // Binding a statement that doesn't exist
    writer.WriteBindCommand(portal_name, "FakeStatementName", {}, {&param1}, {});
    io_socket->FlushAllWrites();
    network::ManualPacketUtil::ReadUntilMessageOrClose(io_socket, network::NetworkMessageType::PG_ERROR_RESPONSE);
  }

  {
    // Wrong number of format codes
    writer.WriteBindCommand(portal_name, stmt_name, {0, 0, 0, 0, 0}, {&param1}, {});
    io_socket->FlushAllWrites();
    network::ManualPacketUtil::ReadUntilMessageOrClose(io_socket, network::NetworkMessageType::PG_ERROR_RESPONSE);
  }

  {
    // Wrong number of parameters
    auto param2 = std::vector<char>({'f', 'a', 'k', 'e'});
    writer.WriteBindCommand(portal_name, stmt_name, {}, {&param1, &param2}, {});
    io_socket->FlushAllWrites();
    network::ManualPacketUtil::ReadUntilMessageOrClose(io_socket, network::NetworkMessageType::PG_ERROR_RESPONSE);
  }

  writer.WriteBindCommand(portal_name, stmt_name, {}, {&param1}, {});
  io_socket->FlushAllWrites();
  network::ManualPacketUtil::ReadUntilMessageOrClose(io_socket, network::NetworkMessageType::PG_BIND_COMPLETE);

  {
    // Describe a statement and a portal that doesn't exist
    writer.WriteDescribeCommand(network::DescribeCommandObjectType::STATEMENT, "FakeStatementName");
    io_socket->FlushAllWrites();
    network::ManualPacketUtil::ReadUntilMessageOrClose(io_socket, network::NetworkMessageType::PG_ERROR_RESPONSE);

    writer.WriteDescribeCommand(network::DescribeCommandObjectType::PORTAL, "FakePortalName");
    io_socket->FlushAllWrites();
    network::ManualPacketUtil::ReadUntilMessageOrClose(io_socket, network::NetworkMessageType::PG_ERROR_RESPONSE);
  }

  {
    // Execute a portal that doesn't exist
    writer.WriteExecuteCommand("FakePortal", 0);
    io_socket->FlushAllWrites();
    network::ManualPacketUtil::ReadUntilMessageOrClose(io_socket, network::NetworkMessageType::PG_ERROR_RESPONSE);
  }

  {
    // A bad describe request
    writer.BeginPacket(network::NetworkMessageType::PG_DESCRIBE_COMMAND)
        .AppendRawValue('?')
        .AppendString(stmt_name)
        .EndPacket();
    io_socket->FlushAllWrites();
    network::ManualPacketUtil::ReadUntilMessageOrClose(io_socket, network::NetworkMessageType::PG_ERROR_RESPONSE);
  }

  network::ManualPacketUtil::TerminateConnection(io_socket->GetSocketFd());
  io_socket->Close();
}

}  // namespace terrier
