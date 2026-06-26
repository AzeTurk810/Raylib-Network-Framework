#pragma once

#include "../Core/Client.h"
#include "../Core/Packet.h"
#include "../Core/Server.h"
#include "../Types.h"
#include <any>
#include <functional>
#include <string>
#include <unordered_map>

namespace rnf {

// NOTE: Packet ID for variable synchronization.
enum : PacketID { PKT_VAR_SYNC = 101 };

//--------------------------------------------------//
//              SERVER-SIDE SYNC                    //
//--------------------------------------------------//
class ServerVariableSync {
public:
  explicit ServerVariableSync(Server &server) : m_server(server) {}

  // NOTE: Tracks a variable by its pointer. Only trivial types are supported.
  template <typename T>
  void track(ClientID id, const std::string &name, T *ptr) {
    static_assert(std::is_trivially_copyable_v<T>,
                  "VariableSync: only trivial types allowed");
    std::string key = std::to_string(id) + ":" + name;
    m_vars[key] = VarEntry{id, name,
                           [ptr]() -> Buffer {
                             Buffer buf(sizeof(T));
                             std::memcpy(buf.data(), ptr, sizeof(T));
                             return buf;
                           },
                           Buffer(sizeof(T), 0)};
  }

  // NOTE: Stops tracking a specific variable.
  void untrack(ClientID id, const std::string &name) {
    m_vars.erase(std::to_string(id) + ":" + name);
  }

  // NOTE: Checks if tracked variables have changed and broadcasts updates.
  void update() {
    for (auto &[key, entry] : m_vars) {
      Buffer current = entry.read();
      if (current != entry.lastValue) {
        Packet p(PKT_VAR_SYNC);
        p.write(entry.ownerId);
        p.write<u16>(static_cast<u16>(entry.name.size()));
        for (char c : entry.name)
          p.write(c);
        p.write<u16>(static_cast<u16>(current.size()));
        for (Byte b : current)
          p.write(b);

        m_server.broadcast(p, Delivery::Reliable);
        entry.lastValue = current;
      }
    }
  }

private:
  struct VarEntry {
    ClientID ownerId;
    std::string name;
    std::function<Buffer()> read;
    Buffer lastValue;
  };
  Server &m_server;
  std::unordered_map<std::string, VarEntry> m_vars;
};

//--------------------------------------------------//
//              CLIENT-SIDE SYNC                    //
//--------------------------------------------------//
class ClientVariableSync {
public:
  explicit ClientVariableSync(Client &client) : m_client(client) {
    client.onReceive([this](Packet p) {
      if (p.id() != PKT_VAR_SYNC)
        return;
      ClientID owner = p.read<ClientID>();
      u16 nameLen = p.read<u16>();
      std::string name;
      for (u16 i = 0; i < nameLen; i++)
        name += p.read<char>();

      u16 dataLen = p.read<u16>();
      Buffer data(dataLen);
      for (u16 i = 0; i < dataLen; i++)
        data[i] = p.read<Byte>();

      std::string key = std::to_string(owner) + ":" + name;
      auto it = m_handlers.find(key);
      if (it != m_handlers.end())
        it->second(owner, data);
    });
  }

  // NOTE: Registers a callback to be triggered when a variable updates.
  template <typename T>
  void onUpdate(ClientID ownerId, const std::string &name,
                std::function<void(ClientID, T)> cb) {
    std::string key = std::to_string(ownerId) + ":" + name;
    m_handlers[key] = [cb](ClientID id, const Buffer &buf) {
      T val;
      std::memcpy(&val, buf.data(), sizeof(T));
      cb(id, val);
    };
  }

private:
  Client &m_client;
  std::unordered_map<std::string, std::function<void(ClientID, const Buffer &)>>
      m_handlers;
};

} // namespace rnf
