#pragma once

#include "../Core/Client.h"
#include "../Core/Packet.h"
#include "../Core/Server.h"
#include "../Types.h"
#include <functional>
#include <string>
#include <unordered_map>
#include <vector>

namespace rnf {

// NOTE: Packet IDs for lobby synchronization.
enum : PacketID {
  PKT_LOBBY_JOIN = 200,
  PKT_LOBBY_LEAVE = 201,
  PKT_LOBBY_READY = 202,
  PKT_LOBBY_STATE = 203, // Server -> Clients: Full lobby list update
  PKT_LOBBY_START = 204  // Server -> Clients: Transition to game state
};

using OnAllReadyCallback = std::function<void()>;
using OnGameStartCallback = std::function<void()>;
using OnLobbyUpdateCallback =
    std::function<void(const std::vector<PlayerInfo> &)>;

//--------------------------------------------------//
//               SERVER-SIDE LOBBY                  //
//--------------------------------------------------//
class ServerLobby {
public:
  explicit ServerLobby(Server &server) : m_server(server) {
    server.onConnect([this](ClientID id) {
      PlayerInfo info;
      info.id = id;
      info.name = "Player" + std::to_string(id);
      info.ready = false;
      m_players[id] = info;
      broadcastState();
    });

    server.onDisconnect([this](ClientID id, DisconnectReason) {
      m_players.erase(id);
      broadcastState();
    });

    server.onReceive([this](ClientID id, Packet p) {
      if (p.id() == PKT_LOBBY_READY) {
        bool ready = p.read<bool>();
        if (m_players.count(id)) {
          m_players[id].ready = ready;
          broadcastState();
          checkAllReady();
        }
      }
      if (p.id() == PKT_LOBBY_JOIN) {
        u16 nameLen = p.read<u16>();
        std::string name;
        for (u16 i = 0; i < nameLen; i++)
          name += p.read<char>();
        if (m_players.count(id))
          m_players[id].name = name;
        broadcastState();
      }
    });
  }

  void setMaxPlayers(u32 max) { m_maxPlayers = max; }
  void onAllReady(OnAllReadyCallback cb) { m_onAllReady = cb; }

  void startGame() {
    Packet p(PKT_LOBBY_START);
    m_server.broadcast(p, Delivery::Reliable);
  }

  const std::unordered_map<ClientID, PlayerInfo> &players() const {
    return m_players;
  }
  u32 playerCount() const { return static_cast<u32>(m_players.size()); }
  bool isFull() const { return playerCount() >= m_maxPlayers; }

private:
  void broadcastState() {
    Packet p(PKT_LOBBY_STATE);
    p.write<u32>(static_cast<u32>(m_players.size()));
    for (auto &[id, info] : m_players) {
      p.write(info.id);
      p.write(info.ready);
      p.write<u16>(static_cast<u16>(info.name.size()));
      for (char c : info.name)
        p.write(c);
    }
    m_server.broadcast(p, Delivery::Reliable);
  }

  void checkAllReady() {
    if (m_players.empty())
      return;
    for (auto &[id, info] : m_players)
      if (!info.ready)
        return;
    if (m_onAllReady)
      m_onAllReady();
  }

  Server &m_server;
  u32 m_maxPlayers = 8;
  std::unordered_map<ClientID, PlayerInfo> m_players;
  OnAllReadyCallback m_onAllReady;
};

//--------------------------------------------------//
//               CLIENT-SIDE LOBBY                  //
//--------------------------------------------------//
class ClientLobby {
public:
  explicit ClientLobby(Client &client) : m_client(client) {
    client.onReceive([this](Packet p) {
      if (p.id() == PKT_LOBBY_STATE) {
        u32 count = p.read<u32>();
        m_players.clear();
        for (u32 i = 0; i < count; i++) {
          PlayerInfo info;
          info.id = p.read<ClientID>();
          info.ready = p.read<bool>();
          u16 nameLen = p.read<u16>();
          for (u16 j = 0; j < nameLen; j++)
            info.name += p.read<char>();
          m_players.push_back(info);
        }
        if (m_onUpdate)
          m_onUpdate(m_players);
      }
      if (p.id() == PKT_LOBBY_START) {
        if (m_onGameStart)
          m_onGameStart();
      }
    });
  }

  void join(const std::string &name) {
    Packet p(PKT_LOBBY_JOIN);
    p.write<u16>(static_cast<u16>(name.size()));
    for (char c : name)
      p.write(c);
    m_client.send(p, Delivery::Reliable);
  }

  void setReady(bool ready) {
    Packet p(PKT_LOBBY_READY);
    p.write(ready);
    m_client.send(p, Delivery::Reliable);
  }

  void onUpdate(OnLobbyUpdateCallback cb) { m_onUpdate = cb; }
  void onGameStart(OnGameStartCallback cb) { m_onGameStart = cb; }
  const std::vector<PlayerInfo> &players() const { return m_players; }

private:
  Client &m_client;
  std::vector<PlayerInfo> m_players;
  OnLobbyUpdateCallback m_onUpdate;
  OnGameStartCallback m_onGameStart;
};

} // namespace rnf
