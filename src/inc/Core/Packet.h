#pragma once

#include "../Types.h"
#include <cstring>
#include <stdexcept>

namespace rnf {

class Packet {
public:
  // NOTE: Creates an empty packet.
  Packet() = default;

  // NOTE: Creates a packet with a specific ID.
  explicit Packet(PacketID id) : m_id(id) {}

  // NOTE: Creates a packet from raw buffer data.
  Packet(const Byte *data, u32 size) {
    if (size < sizeof(PacketHeader))
      throw std::runtime_error("Packet: buffer too small");

    PacketHeader header;
    std::memcpy(&header, data, sizeof(PacketHeader));

    m_id = header.id;
    m_data.assign(data + sizeof(PacketHeader),
                  data + sizeof(PacketHeader) + header.size);
  }

  // NOTE: Writes a trivial type into the packet.
  template <typename T> void write(const T &value) {
    static_assert(std::is_trivially_copyable_v<T>,
                  "Packet::write only supports trivial types");
    const Byte *begin = reinterpret_cast<const Byte *>(&value);
    m_data.insert(m_data.end(), begin, begin + sizeof(T));
  }

  // NOTE: Writes a string into the packet.
  void write(const std::string &str) {
    u16 len = static_cast<u16>(str.size());
    write(len);
    const Byte *begin = reinterpret_cast<const Byte *>(str.data());
    m_data.insert(m_data.end(), begin, begin + len);
  }

  // NOTE: Resets the read position to the beginning.
  void resetRead() { m_readPos = 0; }

  // NOTE: Reads a trivial type from the packet.
  template <typename T> T read() {
    static_assert(std::is_trivially_copyable_v<T>,
                  "Packet::read only supports trivial types");
    if (m_readPos + sizeof(T) > m_data.size())
      throw std::runtime_error("Packet::read - data exhausted");
    T value;
    std::memcpy(&value, m_data.data() + m_readPos, sizeof(T));
    m_readPos += sizeof(T);
    return value;
  }

  // NOTE: Reads a string from the packet.
  template <> std::string read<std::string>() {
    u16 len = read<u16>();
    if (m_readPos + len > m_data.size())
      throw std::runtime_error("Packet::read<string> - data exhausted");
    std::string str(reinterpret_cast<const char *>(m_data.data() + m_readPos),
                    len);
    m_readPos += len;
    return str;
  }

  // NOTE: Serializes the packet into a full buffer for transport.
  Buffer serialize() const {
    PacketHeader header;
    header.id = m_id;
    header.size = static_cast<u32>(m_data.size());

    Buffer out(sizeof(PacketHeader) + m_data.size());
    std::memcpy(out.data(), &header, sizeof(PacketHeader));
    std::memcpy(out.data() + sizeof(PacketHeader), m_data.data(),
                m_data.size());
    return out;
  }

  // NOTE: Returns the packet ID.
  PacketID id() const { return m_id; }

  // NOTE: Returns the size of the data payload.
  u32 dataSize() const { return static_cast<u32>(m_data.size()); }

  // NOTE: Returns true if there is no data in the packet.
  bool empty() const { return m_data.empty(); }

private:
  PacketID m_id = 0;
  Buffer m_data;
  u32 m_readPos = 0;
};

} // namespace rnf
