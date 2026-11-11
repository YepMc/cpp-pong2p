# Pong in C++

A small two-player Pong game using C++ and SDL2. No images, fonts, or sound files are needed. The score appears in the window title.

## Controls

| Action | Key |
| --- | --- |
| Left paddle | W / S |
| Right paddle | Up / Down arrows |
| Pause / resume | Space |
| Reset game and scores | R |
| Quit | Escape |

The ball serves automatically after a short delay. Losing window focus pauses the game; press Space to resume. There is no score limit.

## Build on Windows

Install Visual Studio's **Desktop development with C++** tools, CMake, and [vcpkg](https://github.com/microsoft/vcpkg#quick-start-windows). From PowerShell in the vcpkg folder:

```powershell
.\vcpkg install sdl2:x64-windows
```

Then, from the Pong project folder (replace the example vcpkg path with yours):

```powershell
cmake -S . -B build -A x64 -DCMAKE_TOOLCHAIN_FILE="C:/vcpkg/scripts/buildsystems/vcpkg.cmake"
cmake --build build --config Release
.\build\Release\pong.exe
```

vcpkg's normal CMake integration copies the required SDL2 DLL beside the executable. Keep it there if you move the game.

## Build on Linux (Ubuntu / Debian)

```bash
sudo apt install g++ cmake libsdl2-dev
cmake -S . -B build
cmake --build build
./build/pong
```

SDL2 2.0.22 or newer is required. You can also compile directly:

```bash
g++ -std=c++17 -Wall -Wextra main.cpp -o pong $(sdl2-config --cflags --libs)
./pong
```

## How the code works

Everything is in `main.cpp`. `Game` stores the paddles, ball, score, and pause state. `updateGame` moves them and checks collisions. `resetBall` prepares the next serve. The main loop reads input, updates the game, and draws it.

The game reuses the same state every frame instead of storing a growing list of old frames. Movement uses elapsed time, and the loop waits between frames to avoid keeping the CPU constantly busy. These are basic resource controls, not a claim of advanced optimization.

## Upload to GitHub

Create an empty repository, then upload `main.cpp`, `CMakeLists.txt`, `.gitignore`, and this README. Do not upload the build folder. Alternatively, use Git from this folder:

```bash
git init
git add .
git commit -m "Add simple C++ Pong game"
git branch -M main
git remote add origin https://github.com/YOUR_USERNAME/YOUR_REPO.git
git push -u origin main
```

Use your own repository URL in the remote command.
