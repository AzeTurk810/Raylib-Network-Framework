#pragma once

#include "../Types.h"
#include "Packet.h"
#include <functional>

namespace rnf {

// NOTE:THIS FUNCTION FOR server player connection.
// USAGE: server.onConnect([](ClientID id) { ... });
using OnConnectCallback = std::function<void(ClientID)>;

// NOTE:THIS FUNCTION FOR server player disconnection.
// USAGE: server.onDisconnect([](ClientID id, DisconnectReason reason) { ... });
using OnDisconnectCallback = std::function<void(ClientID, DisconnectReason)>;

// NOTE:THIS FUNCTION FOR receiving data from a player on the server.
// USAGE: server.onReceive([](ClientID id, Packet p) { ... });
using OnServerReceiveCallback = std::function<void(ClientID, Packet)>;

// NOTE:THIS FUNCTION FOR client connection to the server.
// USAGE: client.onConnect([] { ... });
using OnClientConnectCallback = std::function<void()>;

// NOTE:THIS FUNCTION FOR client disconnection from the server.
// USAGE: client.onDisconnect([](DisconnectReason reason) { ... });
using OnClientDisconnectCallback = std::function<void(DisconnectReason)>;

// NOTE:THIS FUNCTION FOR receiving data from the server on the client.
// USAGE: client.onReceive([](Packet p) { ... });
using OnClientReceiveCallback = std::function<void(Packet)>;

} // namespace rnf
