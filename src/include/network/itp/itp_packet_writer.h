#pragma once

#include <memory>
#include <string>
#include <vector>

#include "network/packet_writer.h"

namespace terrier::network {
/**
 * Wrapper around an I/O layer WriteQueue to provide ITP-specific
 * helper methods.
 */
class ITPPacketWriter : public PacketWriter {
 public:
  /**
   * Instantiates a new ITPPacketWriter backed by the given WriteQueue
   */
  explicit ITPPacketWriter(const std::shared_ptr<WriteQueue> &write_queue) : PacketWriter(write_queue) {}

  /**
   * Writes a Replication command
   * --------------------------------------------------------------------------------------------------
   * | message type (char) | message id (uint64_t) | data size (uint64_t) | replication data (varlen) |
   * --------------------------------------------------------------------------------------------------
   * @param message_id message id
   * @param replication_data data for replication
   */
  void WriteReplicationCommand(uint64_t message_id, const std::string& replication_data) {
    BeginPacket(NetworkMessageType::ITP_REPLICATION_COMMAND)
      .AppendValue<uint64_t>(message_id)
      .AppendValue<uint64_t>(replication_data.length())
      .AppendString(replication_data)
      .EndPacket();
  }

};

}  // namespace terrier::network
