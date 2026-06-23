#include "raylib.h"
#include <cstdint>

struct Screen {
  const char *title;
  int width, height;

  Screen(const char *_title, int _width, int _height)
      : title(_title), width(_width), height(_height) {}

  void Init() { InitWindow(this->width, this->height, this->title); }
  void Close() { CloseWindow(); };

  bool IsOpen() { return !WindowShouldClose(); }
};

Screen *screen = new Screen("Example", 1000, 800);

int32_t main(int32_t argc, char **argv) {
  screen->Init();

  while (screen->IsOpen()) {
    BeginDrawing();

    ClearBackground(DARKBLUE);

    EndDrawing();
  }

  screen->Close();
  return 0;
}
