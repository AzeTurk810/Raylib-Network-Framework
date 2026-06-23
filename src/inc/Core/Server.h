#pragma once

#include "../Types.h"
#include "Events.h"
#include "Packet.h"
#include <enet/enet.h>
#include <unordered_map>

namespace rnf {

class Server {
public:
  // NOTE: Initializes server instance.
  Server() = default;

  // NOTE: Cleans up and closes the server.
  ~Server() { close(); }

  // NOTE: Starts the server on a specified port with a maximum client limit.
  bool start(Port port, u32 maxClients = 32) {
    if (enet_initialize() != 0)
      return false;
    ENetAddress address;
    address.host = ENET_HOST_ANY;
    address.port = port;
    m_host = enet_host_create(&address, maxClients, 2, 0, 0);
    if (!m_host)
      return false;
    m_running = true;
    return true;
  }

  // NOTE: Stops the server and disconnects all active peers.
  void close() {
    if (!m_host)
      return;
    for (auto &[id, peer] : m_peers)
      enet_peer_disconnect(peer, 0);
    enet_host_flush(m_host);
    enet_host_destroy(m_host);
    enet_deinitialize();
    m_host = nullptr;
    m_peers.clear();
    m_running = false;
  }

  // NOTE: Processes network events. Should be called every frame.
  void poll(u32 timeoutMs = 0) {
    if (!m_host)
      return;
    ENetEvent event;
    while (enet_host_service(m_host, &event, timeoutMs) > 0) {
      switch (event.type) {
      case ENET_EVENT_TYPE_CONNECT: {
        ClientID id = m_nextID++;
        m_peers[id] = event.peer;
        event.peer->data = reinterpret_cast<void *>(static_cast<uintptr_t>(id));
        if (m_onConnect)
          m_onConnect(id);
        break;
      }
      case ENET_EVENT_TYPE_RECEIVE: {
        ClientID id = static_cast<ClientID>(
            reinterpret_cast<uintptr_t>(event.peer->data));
        Packet p(event.packet->data,
                 static_cast<u32>(event.packet->dataLength));
        if (m_onReceive)
          m_onReceive(id, std::move(p));
        enet_packet_destroy(event.packet);
        break;
      }
      case ENET_EVENT_TYPE_DISCONNECT: {
        ClientID id = static_cast<ClientID>(
            reinterpret_cast<uintptr_t>(event.peer->data));
        m_peers.erase(id);
        if (m_onDisconnect)
          m_onDisconnect(id, DisconnectReason::Unknown);
        break;
      }
      default:
        break;
      }
    }
  }

  // NOTE: Sends a packet to a specific client.
  void send(ClientID id, const Packet &packet,
            Delivery delivery = Delivery::Reliable) {
    auto it = m_peers.find(id);
    if (it == m_peers.end())
      return;
    Buffer buf = packet.serialize();
    u32 flags =
        (delivery == Delivery::Reliable) ? ENET_PACKET_FLAG_RELIABLE : 0;
    ENetPacket *ep = enet_packet_create(buf.data(), buf.size(), flags);
    enet_peer_send(it->second, 0, ep);
  }

  // NOTE: Sends a packet to all connected clients.
  void broadcast(const Packet &packet, Delivery delivery = Delivery::Reliable) {
    Buffer buf = packet.serialize();
    u32 flags =
        (delivery == Delivery::Reliable) ? ENET_PACKET_FLAG_RELIABLE : 0;
    ENetPacket *ep = enet_packet_create(buf.data(), buf.size(), flags);
    enet_host_broadcast(m_host, 0, ep);
  }

  // NOTE: Disconnects a specific client.
  void kick(ClientID id, DisconnectReason reason = DisconnectReason::Kicked) {
    auto it = m_peers.find(id);
    if (it == m_peers.end())
      return;
    enet_peer_disconnect(it->second, static_cast<u32>(reason));
  }

  // NOTE: Sets the callback for a new client connection.
  void onConnect(OnConnectCallback cb) { m_onConnect = cb; }

  // NOTE: Sets the callback for a client disconnection.
  void onDisconnect(OnDisconnectCallback cb) { m_onDisconnect = cb; }

  // NOTE: Sets the callback for receiving data from a client.
  void onReceive(OnServerReceiveCallback cb) { m_onReceive = cb; }

  // NOTE: Returns true if the server is running.
  bool isRunning() const { return m_running; }

  // NOTE: Returns the count of currently connected clients.
  u32 clientCount() const { return static_cast<u32>(m_peers.size()); }

private:
  ENetHost *m_host = nullptr;
  bool m_running = false;
  ClientID m_nextID = 1;
  std::unordered_map<ClientID, ENetPeer *> m_peers;
  OnConnectCallback m_onConnect;
  OnDisconnectCallback m_onDisconnect;
  OnServerReceiveCallback m_onReceive;
};

} // namespace rnf
