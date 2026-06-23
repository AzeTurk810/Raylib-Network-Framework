#pragma once

#include <cstdint>
#include <memory>
#include <string>
#include <vector>

namespace rnf {
using i8 = int8_t;

using i16 = int16_t;

using i32 = int32_t;

using i64 = int64_t;

using u8 = uint8_t;

using u16 = uint16_t;

using u32 = uint32_t;

using u64 = uint64_t;

using f32 = float;

using f64 = double;

using ClientID = u32;

using LobbyID = u32;

using PacketID = u16;

using PeerID = u32;

using Port = u16;

//======================================================//
//                    BUFFER TYPES                      //
//======================================================//

// Bir byte məlumat.
using Byte = u8;

// Paket məlumatlarını saxlamaq üçün byte massivi.
using Buffer = std::vector<Byte>;

//======================================================//
//                SMART POINTER HELPERS                 //
//======================================================//

// unique_ptr üçün qısa yazılış.
template <typename T> using Scope = std::unique_ptr<T>;

// shared_ptr üçün qısa yazılış.
template <typename T> using Ref = std::shared_ptr<T>;

//======================================================//
//                  NETWORK ADDRESS                     //
//======================================================//

// Server və ya client ünvanını saxlamaq üçün.
struct Address {
  std::string ip = "127.0.0.1";
  Port port = 0;
};

//======================================================//
//                PACKET DELIVERY MODE                  //
//======================================================//

// Paketin necə göndəriləcəyini göstərir.
enum class Delivery {
  // Paket mütləq çatmalıdır.
  Reliable,

  // Paket itə bilər, amma daha sürətlidir.
  Unreliable
};

//======================================================//
//                CONNECTION STATE                      //
//======================================================//

// Client-in cari bağlantı vəziyyəti.
enum class ConnectionState {
  Disconnected,
  Connecting,
  Connected,
  Disconnecting
};

//======================================================//
//               DISCONNECT REASON                      //
//======================================================//

// Client niyə ayrıldı?
enum class DisconnectReason {
  Unknown,
  UserRequest,
  Timeout,
  ServerClosed,
  Kicked,
  ConnectionLost,
  NetworkError
};

//======================================================//
//                  NETWORK EVENTS                      //
//======================================================//

// Framework daxilində istifadə olunan event növləri.
enum class EventType { None, Connect, Disconnect, Receive, Timeout, Error };

//======================================================//
//                  LOBBY STATE                         //
//======================================================//

// Lobby hansı vəziyyətdədir?
enum class LobbyState { Closed, Waiting, Starting, InGame };

//======================================================//
//                  LOG LEVEL                           //
//======================================================//

// Logger hansı mesajları göstərsin.
enum class LogLevel { Trace, Debug, Info, Warning, Error, Critical, None };

//======================================================//
//                 PACKET PRIORITY                      //
//======================================================//

// Gələcəkdə prioritetli paket sistemi üçün.
enum class PacketPriority { Low, Medium, High, Critical };

//======================================================//
//                  SYNCHRONIZATION                     //
//======================================================//

// Məlumat avtomatik yoxsa əl ilə sinxronizasiya olunsun.
enum class SyncMode { Manual, Automatic };

//======================================================//
//                  THREAD STATE                        //
//======================================================//

// Network thread-in vəziyyəti.
enum class ThreadState { Stopped, Running, Sleeping };

//======================================================//
//                  PACKET HEADER                       //
//======================================================//

// Hər göndərilən paketin əvvəlində yerləşəcək məlumat.
struct PacketHeader {
  PacketID id = 0;

  u32 size = 0;
};

//======================================================//
//                 NETWORK STATISTICS                   //
//======================================================//

// Debug və profiler üçün istifadə olunur.
struct NetworkStats {
  u32 packetsSent = 0;

  u32 packetsReceived = 0;

  u64 bytesSent = 0;

  u64 bytesReceived = 0;

  u32 ping = 0;

  float packetLoss = 0.0f;
};

//======================================================//
//                 PLAYER INFORMATION                   //
//======================================================//

// Lobby və serverdə oyunçu haqqında məlumat.
struct PlayerInfo {
  ClientID id = 0;

  std::string name;

  bool ready = false;
};

//======================================================//
//                FRAMEWORK CONSTANTS                   //
//======================================================//

// Framework adı.
constexpr const char *FrameworkName = "RNF";

} // namespace rnf
