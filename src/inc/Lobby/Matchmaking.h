#pragma once

#include "../Core/Client.h"
#include "../Core/Packet.h"
#include "../Core/Server.h"
#include "../Types.h"
#include <algorithm>
#include <functional>
#include <vector>

namespace rnf {

// NOTE: Packet IDs for matchmaking communication.
enum : PacketID { PKT_MM_JOIN = 220, PKT_MM_LEAVE = 221, PKT_MM_FOUND = 222 };

using OnMatchReadyCallback = std::function<void(std::vector<ClientID>)>;
using OnMatchFoundCallback = std::function<void()>;

//--------------------------------------------------//
//            SERVER-SIDE MATCHMAKING               //
//--------------------------------------------------//
class ServerMatchmaking {
public:
  explicit ServerMatchmaking(Server &server) : m_server(server) {
    server.onDisconnect(
        [this](ClientID id, DisconnectReason) { leaveQueue(id); });
    server.onReceive([this](ClientID id, Packet p) {
      if (p.id() == PKT_MM_JOIN)
        joinQueue(id);
      if (p.id() == PKT_MM_LEAVE)
        leaveQueue(id);
    });
  }

  // NOTE: Sets the number of players required for a match.
  void setPlayersPerMatch(u32 count) { m_playersPerMatch = count; }

  // NOTE: Registers callback for when a match is formed.
  void onMatchReady(OnMatchReadyCallback cb) { m_onMatchReady = cb; }

private:
  void joinQueue(ClientID id) {
    for (auto qid : m_queue)
      if (qid == id)
        return;
    m_queue.push_back(id);
    tryCreateMatch();
  }

  void leaveQueue(ClientID id) {
    m_queue.erase(std::remove(m_queue.begin(), m_queue.end(), id),
                  m_queue.end());
  }

  void tryCreateMatch() {
    if (m_queue.size() < m_playersPerMatch)
      return;

    std::vector<ClientID> matched(m_queue.begin(),
                                  m_queue.begin() + m_playersPerMatch);
    m_queue.erase(m_queue.begin(), m_queue.begin() + m_playersPerMatch);

    // Notify matched clients
    Packet p(PKT_MM_FOUND);
    for (ClientID id : matched)
      m_server.send(id, p, Delivery::Reliable);

    if (m_onMatchReady)
      m_onMatchReady(matched);
  }

  Server &m_server;
  u32 m_playersPerMatch = 2;
  std::vector<ClientID> m_queue;
  OnMatchReadyCallback m_onMatchReady;
};

//--------------------------------------------------//
//            CLIENT-SIDE MATCHMAKING               //
//--------------------------------------------------//
class ClientMatchmaking {
public:
  explicit ClientMatchmaking(Client &client) : m_client(client) {
    client.onReceive([this](Packet p) {
      if (p.id() == PKT_MM_FOUND) {
        m_inQueue = false;
        if (m_onMatchFound)
          m_onMatchFound();
      }
    });
  }

  // NOTE: Joins the matchmaking queue.
  void joinQueue() {
    if (m_inQueue)
      return;
    m_inQueue = true;
    Packet p(PKT_MM_JOIN);
    m_client.send(p, Delivery::Reliable);
  }

  // NOTE: Removes client from the queue.
  void leaveQueue() {
    if (!m_inQueue)
      return;
    m_inQueue = false;
    Packet p(PKT_MM_LEAVE);
    m_client.send(p, Delivery::Reliable);
  }

  // NOTE: Registers callback for when a match is found.
  void onMatchFound(OnMatchFoundCallback cb) { m_onMatchFound = cb; }
  bool isInQueue() const { return m_inQueue; }

private:
  Client &m_client;
  bool m_inQueue = false;
  OnMatchFoundCallback m_onMatchFound;
};

} // namespace rnf
