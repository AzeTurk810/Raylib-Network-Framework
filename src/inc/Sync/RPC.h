#pragma once

#include "../Core/Client.h"
#include "../Core/Packet.h"
#include "../Core/Server.h"
#include "../Types.h"
#include <functional>
#include <string>
#include <unordered_map>

namespace rnf {

// NOTE: Packet ID for RPC calls.
enum : PacketID { PKT_RPC = 102 };

using RPCHandler = std::function<void(ClientID, Packet)>;

//--------------------------------------------------//
//               SERVER-SIDE RPC                    //
//--------------------------------------------------//
class ServerRPC {
public:
  explicit ServerRPC(Server &server) : m_server(server) {
    server.onReceive([this](ClientID id, Packet p) {
      if (p.id() != PKT_RPC)
        return;

      u16 nameLen = p.read<u16>();
      std::string name;
      for (u16 i = 0; i < nameLen; i++)
        name += p.read<char>();

      auto it = m_handlers.find(name);
      if (it != m_handlers.end())
        it->second(id, std::move(p));
    });
  }

  // NOTE: Registers a handler for a remote function call.
  void on(const std::string &name, RPCHandler handler) {
    m_handlers[name] = handler;
  }

  // NOTE: Calls an RPC on a specific client.
  void callClient(ClientID target, const std::string &name,
                  const Packet &args = Packet(0)) {
    Packet p(PKT_RPC);
    p.write<u16>(static_cast<u16>(name.size()));
    for (char c : name)
      p.write(c);

    Buffer data = args.serialize();
    for (Byte b : data)
      p.write(b);
    m_server.send(target, p);
  }

  // NOTE: Calls an RPC on all connected clients.
  void callAll(const std::string &name, const Packet &args = Packet(0)) {
    Packet p(PKT_RPC);
    p.write<u16>(static_cast<u16>(name.size()));
    for (char c : name)
      p.write(c);

    Buffer data = args.serialize();
    for (Byte b : data)
      p.write(b);
    m_server.broadcast(p);
  }

private:
  Server &m_server;
  std::unordered_map<std::string, RPCHandler> m_handlers;
};

//--------------------------------------------------//
//               CLIENT-SIDE RPC                    //
//--------------------------------------------------//
class ClientRPC {
public:
  explicit ClientRPC(Client &client) : m_client(client) {
    client.onReceive([this](Packet p) {
      if (p.id() != PKT_RPC)
        return;

      u16 nameLen = p.read<u16>();
      std::string name;
      for (u16 i = 0; i < nameLen; i++)
        name += p.read<char>();

      auto it = m_handlers.find(name);
      if (it != m_handlers.end())
        it->second(0, std::move(p));
    });
  }

  // NOTE: Registers a handler for a server-side RPC.
  void on(const std::string &name, RPCHandler handler) {
    m_handlers[name] = handler;
  }

  // NOTE: Calls an RPC on the server.
  void call(const std::string &name, const Packet &args = Packet(0)) {
    Packet p(PKT_RPC);
    p.write<u16>(static_cast<u16>(name.size()));
    for (char c : name)
      p.write(c);

    Buffer data = args.serialize();
    for (Byte b : data)
      p.write(b);
    m_client.send(p);
  }

private:
  Client &m_client;
  std::unordered_map<std::string, RPCHandler> m_handlers;
};

} // namespace rnf
