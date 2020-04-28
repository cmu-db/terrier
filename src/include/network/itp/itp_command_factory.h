#pragma once
#include <memory>

#include "common/managed_pointer.h"
#include "network/itp/itp_network_commands.h"

namespace terrier {
namespace network {
struct InputPacket;
}  // namespace network
}  // namespace terrier

#define MAKE_ITP_COMMAND(type) \
  std::unique_ptr<ITPNetworkCommand>(reinterpret_cast<ITPNetworkCommand *>(new type(packet)))

namespace terrier::network {

/**
 * ITPCommandFactory constructs ITPNetworkCommands that parses input packets to API calls
 * for traffic cop
 */
class ITPCommandFactory {
 public:
  /**
   * Convert an ITP packet to command.
   * @param packet the Postgres input packet
   * @return a unique_ptr to the converted command
   */
  virtual std::unique_ptr<ITPNetworkCommand> PacketToCommand(common::ManagedPointer<InputPacket> packet);

  virtual ~ITPCommandFactory() = default;
};

}  // namespace terrier::network
