#pragma once

#include "../Core/Client.h"
#include "../Core/Packet.h"
#include "../Core/Serialization.h"
#include "../Core/Server.h"
#include "../Types.h"
#include <functional>
#include <raylib.h>
#include <unordered_map>

namespace rnf {

// NOTE: Packet ID for synchronization.
enum : PacketID { PKT_TRANSFORM_SYNC = 100 };

struct TransformState {
  Vector2 position = {0, 0};
  float rotation = 0.0f;
};

// NOTE: Callback when another player's transform changes.
using OnTransformUpdate = std::function<void(ClientID, Vector2, float)>;

//--------------------------------------------------//
//              SERVER-SIDE SYNC                    //
//--------------------------------------------------//
class ServerTransformSync {
public:
  explicit ServerTransformSync(Server &server) : m_server(server) {}

  // NOTE: Tracks pointers to player position and rotation.
  void track(ClientID id, Vector2 *position, float *rotation) {
    m_tracked[id] = {position, rotation, {}};
  }

  // NOTE: Stops tracking a specific client.
  void untrack(ClientID id) { m_tracked.erase(id); }

  // NOTE: Checks for changes and broadcasts updates to all clients.
  void update() {
    for (auto &[id, entry] : m_tracked) {
      Vector2 pos = *entry.position;
      float rot = *entry.rotation;

      if (pos.x != entry.last.position.x || pos.y != entry.last.position.y ||
          rot != entry.last.rotation) {

        Packet p(PKT_TRANSFORM_SYNC);
        p.write(id);
        serialize(p, pos);
        p.write(rot);

        m_server.broadcast(p, Delivery::Unreliable);

        entry.last.position = pos;
        entry.last.rotation = rot;
      }
    }
  }

private:
  struct TrackedEntry {
    Vector2 *position;
    float *rotation;
    TransformState last;
  };
  Server &m_server;
  std::unordered_map<ClientID, TrackedEntry> m_tracked;
};

//--------------------------------------------------//
//              CLIENT-SIDE SYNC                    //
//--------------------------------------------------//
class ClientTransformSync {
public:
  explicit ClientTransformSync(Client &client) : m_client(client) {
    client.onReceive([this](Packet p) {
      if (p.id() != PKT_TRANSFORM_SYNC)
        return;
      ClientID id = p.read<ClientID>();
      Vector2 pos = deserialize<Vector2>(p);
      float rot = p.read<float>();
      if (m_onUpdate)
        m_onUpdate(id, pos, rot);
    });
  }

  // NOTE: Sets the callback for transform updates.
  void onUpdate(OnTransformUpdate cb) { m_onUpdate = cb; }

private:
  Client &m_client;
  OnTransformUpdate m_onUpdate;
};

} // namespace rnf
