# cpp-transport-catalogue
Финальный проект: транспортный справочник


## Build

```bash
cd transport-catalogue
clang++ -std=c++20 -Wall -Wextra -Wpedantic -O2 *.cpp -o transport_catalogue
./transport_catalogue
```

```bash
mkdir build && cd build
cmake ..
cmake --build .
```

```bash
find ./transport-catalogue -iname '*.h' -o -iname '*.cpp' | xargs clang-format -i
```