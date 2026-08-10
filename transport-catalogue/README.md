# Transport Catalogue (JSON I/O stage)

A console program that builds an in-memory database of bus stops and routes
from JSON, then answers statistics queries about routes and stops, also in
JSON. Reads from stdin, writes to stdout — no external dependencies.

## Module map (as recommended by the assignment)

```
geo                  -- coordinates + great-circle distance
domain                -- shared entity-support types (currently just hashers;
                          see "Extend" below)
transport_catalogue    -- the database: stops, buses, distances, queries
request_handler        -- Facade in front of the catalogue
json                   -- generic JSON parser/printer (no app knowledge)
json_reader             -- translates JSON <-> transport_catalogue calls
main                   -- wires it all together
svg / map_renderer     -- stubs, reserved for the SVG map-rendering stage
```

Dependency direction matches the brief: `json` knows nothing about buses;
`json_reader` depends on `domain`/`transport_catalogue`, not the other way
around.

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

Both were exercised while putting this together (CMake via the same flags
the CMakeLists uses — the sandbox here doesn't have `cmake` installed, so
the direct-g++ path is the one actually run end-to-end; the CMakeLists is
a standard, minimal setup that mirrors it).

Requires a C++20 compiler (only for `<format>`, which is included but not
yet used — safe to drop the include and build with `-std=c++17` if your
compiler doesn't have it).

## Run

```bash
./transport_catalogue < tests/example1_input.json
```

Or on Windows-style redirection, same idea:
```bash
transport_catalogue.exe <input.json >output.json
```

`tests/example1_input.json` is the worked example from the assignment
itself; running it reproduces the assignment's expected output exactly
(verified below). `tests/example2_input.json` is an extra case covering a
round-trip route, a stop with no buses through it, and two "not found"
lookups — the branches the first example doesn't exercise.

## What was already correct

The core logic — two-pass `base_requests` handling (stops first, then
distances/routes, so order in the array doesn't matter), round-trip vs.
out-and-back stop expansion, unique-stop counting, `curvature` as
road-length ÷ geographic-length, distance fallback to the reverse direction
when only one way is specified, sorted bus lists per stop, and `"not
found"` handling — was already implemented correctly. Running the
assignment's own example reproduces its expected output field-for-field:

```json
[
    {"buses": ["114"], "request_id": 1},
    {"curvature": 1.23199, "request_id": 2, "route_length": 1700, "stop_count": 3, "unique_stop_count": 2}
]
```

## What I fixed

1. **`transport_catalogue.h` didn't compile** — it uses `uint64_t`/`int64_t`
   but never includes `<cstdint>`. It likely worked on whatever compiler/
   stdlib combination it was written against (which pulled the header in
   transitively) but failed outright here. Added the include.
2. **`json.cpp`: undefined behavior on non-ASCII bytes** — `std::isspace`/
   `std::isdigit` were called with plain `char`, which is signed on most
   platforms; a byte from a multi-byte UTF-8 sequence (e.g. Cyrillic stop
   names, which the assignment uses in its own examples) is negative as a
   `char` and technically undefined behavior when passed to those
   functions. Wrapped the calls in `static_cast<unsigned char>(...)`. In
   practice this couldn't misfire on valid JSON here — non-ASCII bytes only
   ever occur inside quoted strings, which this parser copies byte-for-byte
   without classifying them — but it's a one-line fix for a real portability
   footgun, so worth doing.

Nothing else needed to change to satisfy this stage of the assignment.

## What I'd extend first

1. **SVG map rendering** — `svg.h/.cpp` and `map_renderer.h/.cpp` are
   already stubbed out and wired for exactly this; it's the explicit next
   stage per the assignment text. `map_renderer` would take a
   `render_settings` block from `base_requests`, lay out stops/routes, and
   hand `RequestHandler` a `MapAsSvg`-style stat request.
2. **Unit tests** — the assignment's own hint calls this out directly. Good
   first targets: `geo::ComputeDistance` (known coordinate pairs with hand-
   computed distances), the JSON parser (nesting, escapes, the exact
   whitespace/key-order equivalence the spec demonstrates), and
   `TransportCatalogue` in isolation (unique-stop counting, distance
   fallback, not-found paths) — all pure logic, no stdin/stdout needed, so
   cheap to test without a harness.
3. **Move `Stop`/`Bus` into `domain.h`/`.cpp`** — the assignment's suggested
   module map puts entity classes in `domain`; right now they live in
   `transport_catalogue.h` and `domain.h` only holds hash helpers. Purely
   organizational, no behavior change, but worth it if the course grades
   against that structure.
4. **JSON parser edge cases** — no `\uXXXX` escape support, and malformed
   input throws `std::runtime_error` uncaught (crashes rather than
   producing a JSON error response). Not needed for this assignment's
   "valid input guaranteed" contract, but relevant if you ever point this
   parser at untrusted input.
