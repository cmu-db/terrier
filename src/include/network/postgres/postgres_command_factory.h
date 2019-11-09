#pragma once
#include <memory>
#include "network/postgres/postgres_network_commands.h"

#define MAKE_POSTGRES_COMMAND(type) reinterpret_cast<PostgresNetworkCommand *>(new type(packet))

namespace terrier::network {

/**
 * PostgresCommandFactory constructs PostgresNetworkCommands that parses input packets to API calls
 * into traffic cop
 */
class PostgresCommandFactory {
 public:
  /**
   * Convert a Postgres packet to command.
   * @param packet the Postgres input packet
   * @return a raw pointer to the converted command
   */
  virtual PostgresNetworkCommand *PacketToCommand(InputPacket *packet);

  virtual ~PostgresCommandFactory() = default;
};

}  // namespace terrier::network
