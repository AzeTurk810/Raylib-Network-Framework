#pragma once

#include "../Types.h"
#include "Events.h"
#include "Packet.h"
#include <enet/enet.h>
#include <string>

namespace rnf {

class Client {
public:
  // NOTE: Initializes client instance.
  Client() = default;

  // NOTE: Cleans up and disconnects upon destruction.
  ~Client() { disconnect(); }

  // NOTE: Connects to a server at the specified IP and port.
  bool connect(const std::string &ip, Port port, u32 timeoutMs = 5000) {
    if (enet_initialize() != 0)
      return false;
    m_host = enet_host_create(nullptr, 1, 2, 0, 0);
    if (!m_host)
      return false;
    ENetAddress address;
    enet_address_set_host(&address, ip.c_str());
    address.port = port;
    m_peer = enet_host_connect(m_host, &address, 2, 0);
    if (!m_peer)
      return false;

    ENetEvent event;
    if (enet_host_service(m_host, &event, timeoutMs) > 0 &&
        event.type == ENET_EVENT_TYPE_CONNECT) {
      m_state = ConnectionState::Connected;
      if (m_onConnect)
        m_onConnect();
      return true;
    }
    enet_peer_reset(m_peer);
    m_peer = nullptr;
    m_state = ConnectionState::Disconnected;
    return false;
  }

  // NOTE: Disconnects from the server and cleans up resources.
  void disconnect() {
    if (!m_peer || m_state == ConnectionState::Disconnected)
      return;
    m_state = ConnectionState::Disconnecting;
    enet_peer_disconnect(m_peer, 0);
    ENetEvent event;
    while (enet_host_service(m_host, &event, 3000) > 0) {
      if (event.type == ENET_EVENT_TYPE_RECEIVE)
        enet_packet_destroy(event.packet);
      else if (event.type == ENET_EVENT_TYPE_DISCONNECT)
        break;
    }
    if (m_host) {
      enet_host_destroy(m_host);
      enet_deinitialize();
      m_host = nullptr;
    }
    m_peer = nullptr;
    m_state = ConnectionState::Disconnected;
  }

  // NOTE: Processes network events. Should be called every frame.
  void poll(u32 timeoutMs = 0) {
    if (!m_host)
      return;
    ENetEvent event;
    while (enet_host_service(m_host, &event, timeoutMs) > 0) {
      switch (event.type) {
      case ENET_EVENT_TYPE_RECEIVE: {
        Packet p(event.packet->data,
                 static_cast<u32>(event.packet->dataLength));
        if (m_onReceive)
          m_onReceive(std::move(p));
        enet_packet_destroy(event.packet);
        break;
      }
      case ENET_EVENT_TYPE_DISCONNECT: {
        m_state = ConnectionState::Disconnected;
        m_peer = nullptr;
        if (m_onDisconnect)
          m_onDisconnect(DisconnectReason::Unknown);
        break;
      }
      default:
        break;
      }
    }
  }

  // NOTE: Sends a packet to the server.
  void send(const Packet &packet, Delivery delivery = Delivery::Reliable) {
    if (!m_peer)
      return;
    Buffer buf = packet.serialize();
    u32 flags =
        (delivery == Delivery::Reliable) ? ENET_PACKET_FLAG_RELIABLE : 0;
    ENetPacket *ep = enet_packet_create(buf.data(), buf.size(), flags);
    enet_peer_send(m_peer, 0, ep);
  }

  // NOTE: Sets the callback for successful connection.
  void onConnect(OnClientConnectCallback cb) { m_onConnect = cb; }

  // NOTE: Sets the callback for disconnection.
  void onDisconnect(OnClientDisconnectCallback cb) { m_onDisconnect = cb; }

  // NOTE: Sets the callback for receiving data from the server.
  void onReceive(OnClientReceiveCallback cb) { m_onReceive = cb; }

  // NOTE: Checks if the client is currently connected.
  bool isConnected() const { return m_state == ConnectionState::Connected; }

  // NOTE: Returns the current connection state.
  ConnectionState state() const { return m_state; }

private:
  ENetHost *m_host = nullptr;
  ENetPeer *m_peer = nullptr;
  ConnectionState m_state = ConnectionState::Disconnected;
  OnClientConnectCallback m_onConnect;
  OnClientDisconnectCallback m_onDisconnect;
  OnClientReceiveCallback m_onReceive;
};

} // namespace rnf
