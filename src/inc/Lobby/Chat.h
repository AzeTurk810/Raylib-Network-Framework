#pragma once

#include "../Core/Client.h"
#include "../Core/Packet.h"
#include "../Core/Server.h"
#include "../Types.h"
#include <functional>
#include <string>
#include <vector>

namespace rnf {

// NOTE: Packet ID for chat communication.
enum : PacketID { PKT_CHAT = 210 };

struct ChatMessage {
  ClientID senderId = 0;
  std::string senderName;
  std::string text;
};

using OnMessageCallback =
    std::function<void(const std::string &sender, const std::string &msg)>;

//--------------------------------------------------//
//               SERVER-SIDE CHAT                   //
//--------------------------------------------------//
class ServerChat {
public:
  explicit ServerChat(Server &server) : m_server(server) {
    server.onReceive([this](ClientID id, Packet p) {
      if (p.id() != PKT_CHAT)
        return;

      u16 nameLen = p.read<u16>();
      std::string name;
      for (u16 i = 0; i < nameLen; i++)
        name += p.read<char>();

      u16 msgLen = p.read<u16>();
      std::string msg;
      for (u16 i = 0; i < msgLen; i++)
        msg += p.read<char>();

      // Store in local history
      m_history.push_back({id, name, msg});
      if (m_history.size() > 100)
        m_history.erase(m_history.begin());

      // Broadcast to all clients
      Packet out(PKT_CHAT);
      out.write<u16>(static_cast<u16>(name.size()));
      for (char c : name)
        out.write(c);
      out.write<u16>(static_cast<u16>(msg.size()));
      for (char c : msg)
        out.write(c);

      m_server.broadcast(out, Delivery::Reliable);
    });
  }

  // NOTE: Allows the server to broadcast a message (e.g., system
  // announcements).
  void sendAs(const std::string &name, const std::string &msg) {
    m_history.push_back({0, name, msg});
    Packet out(PKT_CHAT);
    out.write<u16>(static_cast<u16>(name.size()));
    for (char c : name)
      out.write(c);
    out.write<u16>(static_cast<u16>(msg.size()));
    for (char c : msg)
      out.write(c);
    m_server.broadcast(out, Delivery::Reliable);
  }

  const std::vector<ChatMessage> &history() const { return m_history; }

private:
  Server &m_server;
  std::vector<ChatMessage> m_history;
};

//--------------------------------------------------//
//               CLIENT-SIDE CHAT                   //
//--------------------------------------------------//
class ClientChat {
public:
  explicit ClientChat(Client &client, std::string playerName)
      : m_client(client), m_name(std::move(playerName)) {

    client.onReceive([this](Packet p) {
      if (p.id() != PKT_CHAT)
        return;

      u16 nameLen = p.read<u16>();
      std::string name;
      for (u16 i = 0; i < nameLen; i++)
        name += p.read<char>();

      u16 msgLen = p.read<u16>();
      std::string msg;
      for (u16 i = 0; i < msgLen; i++)
        msg += p.read<char>();

      m_history.push_back({0, name, msg});
      if (m_history.size() > 100)
        m_history.erase(m_history.begin());

      if (m_onMessage)
        m_onMessage(name, msg);
    });
  }

  // NOTE: Sends a chat message to the server.
  void send(const std::string &msg) {
    Packet p(PKT_CHAT);
    p.write<u16>(static_cast<u16>(m_name.size()));
    for (char c : m_name)
      p.write(c);
    p.write<u16>(static_cast<u16>(msg.size()));
    for (char c : msg)
      p.write(c);
    m_client.send(p, Delivery::Reliable);
  }

  void onMessage(OnMessageCallback cb) { m_onMessage = cb; }
  const std::vector<ChatMessage> &history() const { return m_history; }

private:
  Client &m_client;
  std::string m_name;
  std::vector<ChatMessage> m_history;
  OnMessageCallback m_onMessage;
};

} // namespace rnf
