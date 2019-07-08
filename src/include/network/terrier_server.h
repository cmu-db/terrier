#pragma once

#include <arpa/inet.h>
#include <event2/buffer.h>
#include <event2/bufferevent.h>
#include <event2/event.h>
#include <event2/listener.h>
#include <netinet/tcp.h>
#include <pthread.h>
#include <sys/file.h>
#include <csignal>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <memory>
#include <vector>
#include "common/dedicated_thread_owner.h"
#include "common/exception.h"
#include "common/notifiable_task.h"
#include "network/connection_dispatcher_task.h"
#include "network/connection_handle_factory.h"
#include "network/network_types.h"

namespace terrier::network {

/**
 * TerrierServer is the entry point of the network layer
 */
class TerrierServer : public common::DedicatedThreadOwner {
 public:
  /**
   * @brief Constructs a new TerrierServer instance.
   *
   * Note that SettingsManager must already be initialized when this constructor
   * is called.
   */
  explicit TerrierServer(common::ManagedPointer<ProtocolInterpreter::Provider> protocol_provider,
                         common::ManagedPointer<ConnectionHandleFactory> connection_handle_factory,
                         common::ManagedPointer<common::DedicatedThreadRegistry> thread_registry);

  ~TerrierServer() override = default;

  /**
   * @brief Configure the server to spin up all its threads and start listening
   * on the configured port.
   *
   * This is separated from the main loop primarily for testing purposes, as we
   * need to wait for the server
   * to start listening on the port before the rest of the test. All
   * event-related settings are also performed
   * here. Since libevent reacts to events fired before the event loop as well,
   * all interactions to the server
   * after this function returns is guaranteed to be handled. For non-testing
   * purposes, you can chain the functions,
   * e.g.:
   *
   *   server.SetupServer().ServerLoop();
   *
   * @return self-reference for chaining
   */
  TerrierServer &SetupServer();

  /**
   * Break from the server loop and exit all network handling threads.
   */
  void Close();

  /**
   * Set listen port to a new port
   * @param new_port
   */
  void SetPort(uint16_t new_port);

 private:
  bool OnThreadRemoval(common::ManagedPointer<common::DedicatedThreadTask> task) override {
    return true;
  }  // TODO(Matt): somewhere there's probably a stronger assertion to be made about the state of the server and if
     // threads can be safely taken away, but I don't understand the networking stuff well enough to say for sure what
     // that assertion is

  // For logging purposes
  // static void LogCallback(int severity, const char *msg);

  uint16_t port_;           // port number
  int listen_fd_ = -1;      // server socket fd that TerrierServer is listening on
  size_t max_connections_;  // maximum number of connections

  common::ManagedPointer<ConnectionHandleFactory> connection_handle_factory_;
  common::ManagedPointer<ProtocolInterpreter::Provider> provider_;
  common::ManagedPointer<ConnectionDispatcherTask> dispatcher_task_;
};
}  // namespace terrier::network
