# Raylib Network Framework (RNF)

A simple, clean networking framework for building online multiplayer games with Raylib. Built on top of ENet (reliable UDP), designed so the API stays out of your way.

---

## How It Works — The Big Picture

```
Your Game Code
      │
      ▼
  Server / Client          ← you call .start() .connect() .poll() .send()
      │
      ▼
   Packet                  ← data is packed into bytes here
      │
      ▼
  ENet (UDP)               ← actual bytes travel over the network
      │
      ▼
  Packet (other side)      ← bytes are unpacked here
      │
      ▼
  Your Callback            ← onReceive / onConnect fires
```

Everything flows through `Packet`. A `Packet` is just a bag of bytes with an ID. You write data in, send it, the other side reads it out. That's the whole model.

---

## Files Written So Far

### `Types.h`

The foundation. Defines all shared types used across the framework:

| Type | What it is |
|------|-----------|
| `ClientID` | Unique ID for each connected player |
| `PacketID` | ID number that identifies what kind of packet it is |
| `Port` | Network port number |
| `Buffer` | Raw byte array (`std::vector<u8>`) |
| `Delivery` | `Reliable` or `Unreliable` — how a packet is sent |
| `ConnectionState` | `Disconnected` / `Connecting` / `Connected` / `Disconnecting` |
| `DisconnectReason` | Why a client left — timeout, kicked, lost connection, etc. |
| `LogLevel` | Controls how much the logger prints |
| `PacketHeader` | Sits at the front of every packet — contains the ID and size |

---

### `Core/Packet.h`

The data container. Everything you send over the network goes through `Packet`.

**How it works:**

- A `Packet` has an ID (so the receiver knows what it contains) and a byte buffer (the actual data).
- You write values in with `write()`, send it, the other side reads them out with `read<T>()` — in the same order you wrote them.
- Internally it prepends a `PacketHeader` (ID + size) before sending so the receiver can reconstruct it.

```cpp
// Sending side
Packet p(MSG_PLAYER_MOVE);   // MSG_PLAYER_MOVE = 1, your enum
p.write(player.x);
p.write(player.y);
server.send(clientId, p);

// Receiving side
server.onReceive([](ClientID id, Packet p) {
    float x = p.read<float>();
    float y = p.read<float>();
});
```

**Supported types:** anything trivially copyable (`int`, `float`, `bool`, structs without pointers), plus `std::string`.

---

### `Core/Events.h`

Defines the callback types used by `Server` and `Client`. Nothing runs here — it just names the function signatures so the rest of the framework agrees on them.

| Callback | When it fires |
|----------|--------------|
| `OnConnectCallback` | Server: a new player connected |
| `OnDisconnectCallback` | Server: a player left |
| `OnServerReceiveCallback` | Server: a player sent a packet |
| `OnClientConnectCallback` | Client: successfully connected to server |
| `OnClientDisconnectCallback` | Client: disconnected from server |
| `OnClientReceiveCallback` | Client: server sent a packet |

---

### `Core/Server.h`

Listens for incoming connections and manages all connected players.

**Lifecycle:**

```
start(port)  →  poll() every frame  →  close()
```

```cpp
rnf::Server server;

server.onConnect([](ClientID id) {
    // new player joined
});

server.onDisconnect([](ClientID id, DisconnectReason reason) {
    // player left
});

server.onReceive([](ClientID id, Packet p) {
    // player sent us something
});

server.start(7777);

while (!WindowShouldClose()) {
    server.poll();          // process all incoming events this frame
    BeginDrawing();
    // draw your game
    EndDrawing();
}

server.close();
```

**Under the hood:** `poll()` calls `enet_host_service()` once per frame. ENet returns a queue of events — connects, disconnects, received packets. The server loops through them and fires your callbacks. `ClientID`s are assigned by the server sequentially starting at 1 and stored in a map alongside the raw `ENetPeer*`.

**Sending:**

```cpp
server.send(id, packet);              // to one player
server.broadcast(packet);             // to all players
server.kick(id);                      // disconnect a player
```

---

### `Core/Client.h`

Connects to a server and exchanges packets with it.

**Lifecycle:**

```
connect(ip, port)  →  poll() every frame  →  disconnect()
```

```cpp
rnf::Client client;

client.onConnect([] {
    // connected to server
});

client.onDisconnect([](DisconnectReason reason) {
    // lost connection
});

client.onReceive([](Packet p) {
    // server sent us something
});

client.connect("127.0.0.1", 7777);

while (!WindowShouldClose()) {
    client.poll();          // process all incoming events this frame
    BeginDrawing();
    // draw your game
    EndDrawing();
}

client.disconnect();
```

**Under the hood:** `connect()` blocks briefly (up to `timeoutMs`, default 5000ms) waiting for the handshake. After that, `poll()` is non-blocking — it just drains whatever ENet has queued up since last frame.

---

### `Core/Serialization.h`

Extends `Packet` with support for Raylib's built-in types.

Without this, you'd have to manually write each field of a `Vector2`:

```cpp
p.write(v.x);
p.write(v.y);
```

With it:

```cpp
serialize(p, player.position);           // Vector2
auto pos = deserialize<Vector2>(p);
```

**Supported Raylib types:** `Vector2`, `Vector3`, `Vector4`, `Rectangle`, `Color`, `Matrix`.

---

### `Integration/RaylibBackend.h`

Wraps a `Server` or `Client` and ties it into the Raylib game loop automatically so you never forget to call `poll()`.

```cpp
rnf::Server server;
// ... setup callbacks, call server.start(7777) ...

rnf::ServerBackend backend(server);
backend.run([] {
    BeginDrawing();
    ClearBackground(BLACK);
    // draw game
    EndDrawing();
});
// loop ends when WindowShouldClose() is true
```

`run()` does exactly this every frame:

1. `server.poll()` — handle network events
2. call your draw function

If you want to manage the loop yourself, use `backend.pollOnce()` inside your own `while` loop instead.

---

### `Tools/Logger.h`

A lightweight logger. Keeps the last 200 messages in memory (for `DebugOverlay` later) and optionally prints to the terminal.

```cpp
Logger::info("Player connected: " + std::to_string(id));
Logger::warn("Packet arrived late");
Logger::error("Connection dropped");

Logger::setLevel(LogLevel::Warning);   // hide Trace and Info
Logger::setConsoleOutput(false);       // silence the terminal
```

Log entries are stored as `LogEntry` structs with a timestamp, level, and message — ready to be displayed by `DebugOverlay` when that is written.

---

## Dependency Map

```
Types.h
  └── Packet.h
        └── Events.h
              ├── Server.h
              │     └── RaylibBackend.h
              └── Client.h
                    └── RaylibBackend.h

Packet.h + raylib.h
  └── Serialization.h

Logger.h  (standalone, used by everything)
```

---

## What's Next

| Module | What it will do |
|--------|----------------|
| `Sync/TransformSync.h` | Automatically sync position/rotation of game objects |
| `Sync/VariableSync.h` | Sync any variable (health, score, state) with one line |
| `Sync/RPC.h` | Call a function on the remote side like a local function |
| `Sync/Snapshot.h` | Full game state snapshots for rollback / lag compensation |
| `Lobby/Lobby.h` | Room system — create, join, leave game rooms |
| `Lobby/Chat.h` | In-game text chat |
| `Lobby/Matchmaking.h` | Simple matchmaking queue |
| `GUI/DebugOverlay.h` | Raylib overlay showing ping, packet loss, logs in-game |
| `Tools/Profiler.h` | Network performance measurements |
| `Tools/PacketInspector.h` | See every packet in real time while debugging |
