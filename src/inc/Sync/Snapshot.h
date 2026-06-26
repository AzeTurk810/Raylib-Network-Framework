#pragma once

#include "../Core/Client.h"
#include "../Core/Packet.h"
#include "../Core/Serialization.h"
#include "../Core/Server.h"
#include "../Types.h"
#include <functional>
#include <raylib.h>
#include <unordered_map>
#include <vector>

namespace rnf {

// NOTE: Packet ID for snapshot synchronization.
enum : PacketID { PKT_SNAPSHOT = 103 };

template <typename T> class ServerSnapshot {
public:
  explicit ServerSnapshot(Server &server) : m_server(server) {}

  // NOTE: Sets the reader function to retrieve the current game state.
  void setReader(std::function<T()> reader) { m_reader = reader; }

  // NOTE: Sends the current snapshot to a specific client.
  void sendTo(ClientID id) {
    if (!m_reader)
      return;
    T state = m_reader();
    Packet p(PKT_SNAPSHOT);

    p.write<u32>(sizeof(T));
    const Byte *ptr = reinterpret_cast<const Byte *>(&state);
    for (u32 i = 0; i < sizeof(T); i++)
      p.write(ptr[i]);

    m_server.send(id, p, Delivery::Reliable);
  }

  // NOTE: Broadcasts the current snapshot to all connected clients.
  void broadcast() {
    if (!m_reader)
      return;
    T state = m_reader();
    Packet p(PKT_SNAPSHOT);

    p.write<u32>(sizeof(T));
    const Byte *ptr = reinterpret_cast<const Byte *>(&state);
    for (u32 i = 0; i < sizeof(T); i++)
      p.write(ptr[i]);

    m_server.broadcast(p, Delivery::Reliable);
  }

private:
  Server &m_server;
  std::function<T()> m_reader;
};

template <typename T> class ClientSnapshot {
public:
  explicit ClientSnapshot(Client &client) : m_client(client) {
    client.onReceive([this](Packet p) {
      if (p.id() != PKT_SNAPSHOT)
        return;

      u32 size = p.read<u32>();
      if (size != sizeof(T))
        return; // Type mismatch check

      T state;
      Byte *ptr = reinterpret_cast<Byte *>(&state);
      for (u32 i = 0; i < sizeof(T); i++)
        ptr[i] = p.read<Byte>();

      if (m_onReceive)
        m_onReceive(state);
    });
  }

  // NOTE: Registers a callback to handle the received snapshot.
  void onReceive(std::function<void(T)> cb) { m_onReceive = cb; }

private:
  Client &m_client;
  std::function<void(T)> m_onReceive;
};

} // namespace rnf
