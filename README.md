# CppSlim

A C++17 implementation of the [FitNesse SLIM](http://fitnesse.org/FitNesse.UserGuide.WritingAcceptanceTests.SliM) protocol. Write acceptance-test fixtures as plain C++ classes; FitNesse drives them over a TCP socket.

## Prerequisites

| Platform | Toolchain |
|---|---|
| Windows | CMake ≥ 3.17, any C++17 compiler (MinGW GCC or MSVC) |
| Linux / Docker | CMake ≥ 3.17, GCC or Clang, make |

An internet connection is required the first time CMake fetches [GoogleTest](https://github.com/google/googletest).

## Building

### Windows (Ninja)

```bat
cd cppslim
cmake -B build -G Ninja
cmake --build build
```

The executable is `build/CppSlim.exe`.

### Linux / macOS

```sh
cd cppslim
cmake -B build-linux -DCMAKE_BUILD_TYPE=Release
cmake --build build-linux -j4
```

The executable is `build-linux/CppSlim`.

### Docker

A ready-made Ubuntu 22.04 dev image is in `.devcontainer/`:

```sh
docker build -t cppslim-dev .devcontainer/
docker run --rm -v "$PWD/cppslim:/workspace" cppslim-dev bash -c "
    cmake -S /workspace -B /tmp/build-linux -DCMAKE_BUILD_TYPE=Release &&
    cmake --build /tmp/build-linux -j4 &&
    /tmp/build-linux/tests/CppSlimTests"
```

The build directory is placed in `/tmp` inside the container to avoid permission conflicts with the Windows host volume.

### Running the tests

```sh
# Windows
build/tests/CppSlimTests.exe

# Linux / Docker
build-linux/tests/CppSlimTests
```

All 105 tests should pass on both platforms.

---

## Writing a fixture

Include `CppFixtures.h` and derive your class from `SlimFixture<YourClass>`.

```cpp
#include <vector>
#include <string>
#include "CppFixtures.h"

class MyFixture : public SlimFixture<MyFixture> {
public:
    // Constructor always receives a vector<string> of the table's constructor arguments.
    explicit MyFixture(const std::vector<std::string>& args) {}

    // Methods can take and return any supported type (see table below).
    void   setValue(int v)     { value_ = v; }
    int    getValue() const    { return value_; }
    bool   isPositive() const  { return value_ > 0; }
    std::string describe()     { return "value is " + std::to_string(value_); }

private:
    int value_ = 0;
};

// Register the fixture and every method FitNesse can call.
SLIM_FIXTURE(MyFixture)
    SLIM_METHOD(setValue)
    SLIM_METHOD(getValue)
    SLIM_METHOD(isPositive)
    SLIM_METHOD(describe)
SLIM_END
```

### Supported types

`BoundMethod` converts between SLIM's string wire format and native C++ types automatically via `SlimConvert<T>`.

| C++ type | FitNesse value |
|---|---|
| `int` | decimal integer string |
| `long` | decimal integer string |
| `double` | floating-point string (`atof`) |
| `float` | floating-point string |
| `bool` | `"true"` / `"false"` (also accepts `"yes"` and `"1"`) |
| `std::string` | passed through as-is |
| `void` (return) | ignored by FitNesse |

Constructor arguments arrive as `std::vector<std::string>` — convert them manually if needed.

### Signalling errors

Throw any `std::exception` from a method or constructor; CppSlim catches it and reports a `__EXCEPTION__` result to FitNesse:

```cpp
void setDenominator(double d) {
    if (d == 0.0)
        throw std::runtime_error("Cannot divide by zero");
    denominator_ = d;
}
```

To report an error from a `const char*`-returning context, use:

```cpp
return StatementExecutor_FixtureError("something went wrong");
```

---

## Registering a fixture

Open `fixtures/Fixtures.cpp` and add one `SLIM_INCLUDE_FIXTURE` call per fixture class:

```cpp
#include "CppFixtures.h"

void AddFixtures(StatementExecutor* executor) {
    SLIM_INCLUDE_FIXTURE(MyFixture);
    SLIM_INCLUDE_FIXTURE(Division);
    SLIM_INCLUDE_FIXTURE(Count);
    SLIM_INCLUDE_FIXTURE(EmployeePayRecords);
}
```

Then add the corresponding `.cpp` file to the `CppSlim` target in `CMakeLists.txt`:

```cmake
add_executable(CppSlim
    src/main.cpp
    fixtures/Fixtures.cpp
    fixtures/MyFixture.cpp   # <-- add this
    ...
)
```

---

## Wiring to FitNesse

In any FitNesse wiki page, set these four `define` variables before the test tables:

```
!define TEST_SYSTEM {slim}
!define SLIM_PORT   {0}
!define TEST_RUNNER {cppslim/build/CppSlim.exe}
!define COMMAND_PATTERN {%m}
```

- `SLIM_PORT 0` tells FitNesse to pick a free TCP port and pass it as `argv[1]` to the runner.
- `TEST_RUNNER` should be the path to `CppSlim.exe` (or `CppSlim` on Linux) relative to the FitNesse root.
- `COMMAND_PATTERN {%m}` runs the executable directly with the port as its only argument.

### Table types

**Decision table** — one row per set of inputs, checks outputs:

```
|MyFixture                 |
|setValue|getValue?        |
|42      |42               |
|-1      |-1               |
```

**Script table** — imperative sequence of calls:

```
|script|Count   |
|count           |
|check|counter|1|
```

**Query table** — fixture returns a collection of rows:

```
!|query:EmployeePayRecords|
|id       |pay            |
|1        |1000           |
```

---

## CMake targets

| Target | Type | Purpose |
|---|---|---|
| `CppSlimFixtures` | INTERFACE | Headers-only fixture-authoring library. Link your fixture library against this target only — no protocol code needed at compile time. |
| `CppSlimLib` | STATIC | Full protocol/transport implementation. Links `CppSlimFixtures` publicly, so the final executable only needs to link `CppSlimLib`. |
| `Fixtures` | STATIC | Example fixture library (Division, Count, EmployeePayRecords). Links `CppSlimFixtures` only — demonstrates the clean separation. |
| `CppSlim` | executable | Links `CppSlimLib + Fixtures`. |

Typical layout for your own fixture library:

```cmake
add_library(MyFixtures STATIC MyFixture.cpp)
target_link_libraries(MyFixtures PRIVATE CppSlimFixtures)   # compile-time only

add_executable(MyCppSlim src/main.cpp fixtures/Fixtures.cpp)
target_link_libraries(MyCppSlim PRIVATE CppSlimLib MyFixtures)  # link-time
```

---

## Project layout

```
cppslim/
├── include/CppSlim/        # Public headers
│   ├── CppFixtures.h       # SLIM_FIXTURE / SLIM_METHOD / SLIM_END / SLIM_INCLUDE_FIXTURE
│   ├── SlimFixture.h       # SlimFixture<T> CRTP base, BoundMethod<T,Ptr>, SlimConvert<T>
│   ├── SlimList.h          # Wire-format list type used by query fixtures
│   ├── SlimListSerializer.h
│   └── StatementExecutor.h # Low-level fixture registration (used by macros)
├── src/                    # Protocol/transport implementation (CppSlimLib)
├── fixtures/               # Example fixtures + Fixtures.cpp registry (Fixtures target)
└── tests/                  # GoogleTest suite (105 tests)
```

Only `CppFixtures.h` and `SlimFixture.h` are needed for ordinary fixture development. The other headers are available if a fixture needs to build `SlimList` structures manually (see `EmployeePayRecords.cpp`).
