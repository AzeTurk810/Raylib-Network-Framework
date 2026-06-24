#pragma once

#include "../Types.h"
#include "Packet.h"
#include <raylib.h>

namespace rnf {

// NOTE: Serializes a Vector2 to the packet.
inline void serialize(Packet &p, Vector2 v) {
  p.write(v.x);
  p.write(v.y);
}

// NOTE: Serializes a Vector3 to the packet.
inline void serialize(Packet &p, Vector3 v) {
  p.write(v.x);
  p.write(v.y);
  p.write(v.z);
}

// NOTE: Serializes a Vector4 to the packet.
inline void serialize(Packet &p, Vector4 v) {
  p.write(v.x);
  p.write(v.y);
  p.write(v.z);
  p.write(v.w);
}

// NOTE: Serializes a Rectangle to the packet.
inline void serialize(Packet &p, Rectangle r) {
  p.write(r.x);
  p.write(r.y);
  p.write(r.width);
  p.write(r.height);
}

// NOTE: Serializes a Color to the packet.
inline void serialize(Packet &p, Color c) {
  p.write(c.r);
  p.write(c.g);
  p.write(c.b);
  p.write(c.a);
}

// NOTE: Serializes a Matrix to the packet.
inline void serialize(Packet &p, Matrix m) {
  p.write(m.m0);
  p.write(m.m1);
  p.write(m.m2);
  p.write(m.m3);
  p.write(m.m4);
  p.write(m.m5);
  p.write(m.m6);
  p.write(m.m7);
  p.write(m.m8);
  p.write(m.m9);
  p.write(m.m10);
  p.write(m.m11);
  p.write(m.m12);
  p.write(m.m13);
  p.write(m.m14);
  p.write(m.m15);
}

template <typename T> T deserialize(Packet &p);

// NOTE: Deserializes a Vector2 from the packet.
template <> inline Vector2 deserialize<Vector2>(Packet &p) {
  float x = p.read<float>();
  float y = p.read<float>();
  return {x, y};
}

// NOTE: Deserializes a Vector3 from the packet.
template <> inline Vector3 deserialize<Vector3>(Packet &p) {
  float x = p.read<float>();
  float y = p.read<float>();
  float z = p.read<float>();
  return {x, y, z};
}

// NOTE: Deserializes a Vector4 from the packet.
template <> inline Vector4 deserialize<Vector4>(Packet &p) {
  float x = p.read<float>();
  float y = p.read<float>();
  float z = p.read<float>();
  float w = p.read<float>();
  return {x, y, z, w};
}

// NOTE: Deserializes a Rectangle from the packet.
template <> inline Rectangle deserialize<Rectangle>(Packet &p) {
  float x = p.read<float>();
  float y = p.read<float>();
  float w = p.read<float>();
  float h = p.read<float>();
  return {x, y, w, h};
}

// NOTE: Deserializes a Color from the packet.
template <> inline Color deserialize<Color>(Packet &p) {
  u8 r = p.read<u8>();
  u8 g = p.read<u8>();
  u8 b = p.read<u8>();
  u8 a = p.read<u8>();
  return {r, g, b, a};
}

// NOTE: Deserializes a Matrix from the packet.
template <> inline Matrix deserialize<Matrix>(Packet &p) {
  Matrix m;
  m.m0 = p.read<float>();
  m.m1 = p.read<float>();
  m.m2 = p.read<float>();
  m.m3 = p.read<float>();
  m.m4 = p.read<float>();
  m.m5 = p.read<float>();
  m.m6 = p.read<float>();
  m.m7 = p.read<float>();
  m.m8 = p.read<float>();
  m.m9 = p.read<float>();
  m.m10 = p.read<float>();
  m.m11 = p.read<float>();
  m.m12 = p.read<float>();
  m.m13 = p.read<float>();
  m.m14 = p.read<float>();
  m.m15 = p.read<float>();
  return m;
}

} // namespace rnf
