# Stack

A small C++23 learning project comparing two LIFO stack implementations:

- **`Stack<T>`** — a singly linked stack that allocates a node on each push and deletes it on pop.
- **`ArenaStack<T>`** — a stack backed by a fixed-size arena, allocating storage by advancing a byte offset and reusing it by rewinding that offset.

The goal is to understand object lifetime, dynamic allocation, modern target-based CMake, GoogleTest, and Google Benchmark. This is an educational implementation, not a production container library or trading system.

## Requirements

- CMake 4.0 or newer
- A compiler with support for the C++23 features used here
- Ninja (for the commands below)
- Git, to initialize the GoogleTest and Google Benchmark submodules

From the project root, initialize dependencies:

```sh
git submodule update --init --recursive
```

CMake also attempts to update submodules during configuration by default. After initializing them manually, pass `-DUPDATE_SUBMODULES=OFF` to disable that behavior.

## Build and test

Configure a Debug build with tests enabled:

```sh
cmake -S . -B build/debug -G Ninja \
  -DCMAKE_BUILD_TYPE=Debug \
  -DBUILD_TESTING=ON

cmake --build build/debug
ctest --test-dir build/debug --output-on-failure
```

The test executables can also be run directly:

```sh
./build/debug/tests
./build/debug/arena_tests
```

GoogleTest cases from both executables are registered with CTest through `gtest_discover_tests()`.

### Check header self-containment

The public headers are declared using a CMake header file set. To check that each header compiles when included on its own:

```sh
cmake --build build/debug --target all_verify_interface_header_sets
```

This verification target is not part of the default build.

## Benchmarks

Use an optimized build for performance measurements:

```sh
cmake -S . -B build/release -G Ninja \
  -DCMAKE_BUILD_TYPE=Release \
  -DSTACK_BENCHMARKS=ON

cmake --build build/release \
  --target benchmark_stack benchmark_arena_stack

./build/release/benchmark_stack
./build/release/benchmark_arena_stack
```

Repeat measurements or run a specific workload:

```sh
./build/release/benchmark_stack --benchmark_repetitions=5
./build/release/benchmark_arena_stack \
  --benchmark_filter='^BM_real_traffic/10000$' \
  --benchmark_repetitions=5
```

The benchmark workloads cover:

| Workload | Work measured per iteration |
| --- | --- |
| `BM_push_pop` | One push followed by one pop |
| `BM_peek` | Reading the same populated stack's top element |
| `BM_fill_then_empty/N` | Pushing N elements, then popping N elements |
| `BM_real_traffic/N` | Replaying a pregenerated random push/pop schedule, followed by draining the stack |

The random workload uses a fixed seed, prevents pops on an empty stack, and appends enough pops to finish empty. Random-number generation occurs outside the timed loop. Despite its name, `BM_real_traffic` is a synthetic workload, not a model validated against actual order traffic.

### Interpreting results

- Reported times are averages per benchmark iteration. For batch workloads, that means the whole batch, not one stack operation.
- The arena's backing allocation happens before the timed loop; linked-stack node allocation and deletion happen during operations. The comparison is between complete storage designs, not just allocation functions.
- Repeated peeks normally access a hot cache line. Very small timings do not represent arbitrary-memory access latency.
- `DoNotOptimize()` keeps values observable to the compiler. `ClobberMemory()` is a compiler memory barrier, not a CPU-cache flush or thread-synchronization primitive.
- Results depend on optimization, allocator behavior, hardware, and system activity. Average timings do not establish worst-case latency.

## Usage

### Linked stack

```cpp
#include "stack.hpp"

Stack<int> stack;
stack.push(10);
stack.push(20);

if (auto top = stack.peek()) {
    // *top is 20; peek returns a copy without removing it.
}

int output{};
if (stack.pop(output)) {
    // output is 20.
}
```

### Arena-backed stack

```cpp
#include "arena_stack.hpp"

struct Order {
    unsigned int id{};
    unsigned int quantity{};
};

Arena arena{sizeof(Order) * 1024};
ArenaStack<Order> orders{&arena};

orders.push(Order{.id = 1, .quantity = 100});

Order output{};
if (orders.pop(output)) {
    // output contains the removed order.
}
```

Both stacks provide `push()`, `pop(T&)`, `peek()`, and `size()`. An empty pop returns `false` without changing the output; an empty peek returns `std::nullopt`. Neither stack supports copying or moving.

## Arena model and limitations

The arena owns one raw memory block. Allocation reserves aligned bytes; placement new constructs an object in those bytes. Rewinding the offset makes storage reusable but does not clear it or return it to the operating system. The arena destructor releases the backing block.

The current arena stack deliberately relies on a narrow contract:

- `T` must be trivially destructible, enforced by a template constraint.
- The supplied arena pointer must be non-null, and the arena must outlive the stack.
- One stack must exclusively use its arena. Do not interleave unrelated allocations or externally reset the arena while the stack contains elements.
- Allocations use the same element size and alignment. Popping subtracts `sizeof(T)`; it does not generally recover alignment padding for mixed allocations.
- Arena exhaustion throws `std::bad_alloc`; the backing block does not grow.
- Destroying an arena stack does not automatically rewind the arena.
- Trivial destruction does not imply nonthrowing construction. If constructing an element throws after allocation, the current implementation does not roll back the consumed storage.
- Pointers into reclaimed storage must not be used as though the removed elements still exist.

The linked stack uses recursive node destruction. Destroying a sufficiently long remaining chain can exhaust the call stack.

## CMake options

| Option | Default | Purpose |
| --- | --- | --- |
| `BUILD_TESTING` | `ON` | Build and register both test executables |
| `STACK_BENCHMARKS` | `OFF` | Build both benchmark executables |
| `STACK_EXAMPLES` | `OFF` | Build the `examples` executable |
| `UPDATE_SUBMODULES` | `ON` when Git is found | Attempt dependency initialization/update during configuration |

The header-only target is exposed locally as `stack::stack`. Consumers inherit its include directory through the header file set. The project also generates `compile_commands.json` with supported generators such as Ninja.

Generator selection belongs in the configure command or CMake presets, not in `CMakeLists.txt`. An existing build directory cannot switch generators; use a fresh directory when changing generators.
