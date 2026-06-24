#pragma once

#include "../Core/Client.h"
#include "../Core/Server.h"
#include "../Types.h"
#include <functional>
#include <raylib.h>

namespace rnf {

template <typename T> class RaylibBackend {
public:
  // NOTE: Initializes backend with a server or client reference.
  explicit RaylibBackend(T &peer) : m_peer(peer) {}

  // NOTE: Starts the main loop, handling network polling and rendering.
  void run(std::function<void()> drawFn) {
    while (!WindowShouldClose()) {
      m_peer.poll();
      drawFn();
    }
  }

  // NOTE: Manually processes network events; call this at the start of your
  // custom loop.
  void pollOnce() { m_peer.poll(); }

private:
  T &m_peer;
};

// NOTE: Alias for server backend usage.
using ServerBackend = RaylibBackend<Server>;

// NOTE: Alias for client backend usage.
using ClientBackend = RaylibBackend<Client>;

} // namespace rnf
