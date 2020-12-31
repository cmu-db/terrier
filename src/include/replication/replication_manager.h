#pragma once

#include <condition_variable>
#include <mutex>
#include <string>
#include <unordered_map>
#include <utility>

#include "common/managed_pointer.h"
#include "messenger/connection_destination.h"
#include "messenger/messenger.h"

namespace noisepage::storage {
class BufferedLogWriter;
}  // namespace noisepage::storage

namespace noisepage::replication {

class ReplicationManager;

/** Abstraction around a replica. */
class Replica {
  // todo(wan): justify existence? what other methods are specific to communicating with a replica?
 public:
  Replica(common::ManagedPointer<messenger::Messenger> messenger, const std::string &replica_name,
          const std::string &hostname, uint16_t port);

  common::ManagedPointer<messenger::ConnectionId> GetConnectionId() { return common::ManagedPointer(&connection_); }

 private:
  friend ReplicationManager;

  messenger::ConnectionDestination replica_info_;
  messenger::ConnectionId connection_;
  uint64_t last_heartbeat_;  ///< Time (unix epoch) that the replica heartbeat was last successful.
};

/**
 * ReplicationManager is responsible for all aspects of database replication.
 *
 * Housekeeping duties include:
 * - Maintaining a list of all known database replicas and the presumed state of each replica.
 * - Sending periodic heartbeats to other database replicas.
 *
 * The ReplicationManager can be invoked by other components in order to send specific replication-related data.
 * For example:
 * - LogManager may ask the ReplicationManager to replicate a specific log record to all other replicas.
 * - Catalog may ask the ReplicationManager to replicate some specific in-memory data.
 *
 * This is coordinated through MessageType below. The ReplicationManager is aware of all required functionality from
 * the subsystems that the ReplicationManager interfaces with. The ReplicationManager is responsible for invoking
 * the appropriate functions.
 *
 * (this logic is pushed to the ReplicationManager to avoid littering the rest of the codebase with retry/failure etc)
 *
 * All replication communication happens over REPLICATION_DEFAULT_PORT.
 */
class ReplicationManager {
 public:
  enum class MessageType : uint8_t { RESERVED = 0, ACK, HEARTBEAT, REPLICATE_BUFFER };
  /** Milliseconds between replication heartbeats before a replica is declared dead. */
  static constexpr uint64_t REPLICATION_CARDIAC_ARREST_MS = 5000;

  ReplicationManager(common::ManagedPointer<messenger::Messenger> messenger, const std::string &network_identity,
                     uint16_t port, const std::string &replication_hosts_path);

  void ReplicaConnect(const std::string &replica_name, const std::string &hostname, uint16_t port);

  // sync/async seems relatively easy to do in this framework, though I need to fix up the multithreaded block case
  // just capture a bool in the callback and flip it on response, have the cvar wait on that
  // I don't think this should actually send the message, though - it should buffer the message to be sent instead
  void ReplicaSend(const std::string &replica_name, MessageType type, const std::string &msg, bool block);

  void ReplicateBuffer(storage::BufferedLogWriter *buffer);

  /** @return The port that replication is running on. */
  uint16_t GetPort() const { return port_; }

  std::vector<std::string> GetCurrentReplicaList() const {
    std::vector<std::string> replicas;
    replicas.reserve(replicas_.size());
    for (const auto &replica : replicas_) {
      replicas.emplace_back(replica.first);
    }
    return replicas;
  }

 private:
  void EventLoop(common::ManagedPointer<messenger::Messenger> messenger, const messenger::ZmqMessage &msg);
  void ReplicaHeartbeat(const std::string &replica_name);

  void BuildReplicaList(const std::string &replication_hosts_path);

  // todo(wan): division of functionality?'
  // todo(wan): RM would become a dependency to most components

  common::ManagedPointer<messenger::ConnectionId> GetReplicaConnection(const std::string &replica_name);

  common::ManagedPointer<messenger::Messenger> messenger_;
  std::string identity_;                               ///< The identity of this replica.
  uint16_t port_;                                      ///< The port that replication runs on.
  std::unordered_map<std::string, Replica> replicas_;  ///< Replica Name -> Connection ID.
  std::mutex mutex_;
  std::condition_variable cvar_;
};

}  // namespace noisepage::replication