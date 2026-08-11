# cpp-transport-catalogue
Финальный проект: транспортный справочник


## Build

**Option A — direct g++ (fastest for iterating):**
```bash
g++ -std=c++20 -Wall -Wextra -O2 *.cpp -o transport_catalogue
```

**Option B — CMake (what you'd normally use in an IDE / CLion / Visual Studio):**
```bash
mkdir build && cd build
cmake ..
cmake --build .
```