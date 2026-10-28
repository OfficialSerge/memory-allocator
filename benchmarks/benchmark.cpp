#include "stack.hpp"

#include <benchmark/benchmark.h>
#include <cstddef>
#include <cstdint>
#include <random>
#include <vector>

enum class Events { Push, Pop };
enum class Side { Buy, Sell };
enum class OrderType { Limit };

struct Order {
  std::uint64_t id{};
  Side side{Side::Buy};
  OrderType type{OrderType::Limit};
  std::uint64_t quantity{};
};

static void BM_push_pop(benchmark::State& state) {
  Stack<Order> stack;

  Order output{};
  Order input = {.id = 42, .side = Side::Buy, .type = OrderType::Limit, .quantity = 10};

  for (auto _ : state) {
    // force the compiler to not optimize away the value!
    benchmark::DoNotOptimize(input);

    stack.push(input);
    benchmark::DoNotOptimize(stack);

    // acts as a full compiler memory barrier, forcing the compiler to
    // flush pending writes to RAM and reload variables from memory.
    benchmark::ClobberMemory();

    bool popped = stack.pop(output);
    benchmark::DoNotOptimize(popped);
    benchmark::DoNotOptimize(output);
  }
}

static void BM_peek(benchmark::State& state) {
  Stack<Order> stack;
  Order input = {.id = 42, .side = Side::Buy, .type = OrderType::Limit, .quantity = 10};
  benchmark::DoNotOptimize(input);

  stack.push(input);
  benchmark::DoNotOptimize(stack);

  for (auto _ : state) {
    benchmark::ClobberMemory();

    auto out = stack.peek();
    benchmark::DoNotOptimize(out);
  }
}

static void BM_fill_then_empty(benchmark::State& state) {
  const auto num_elements = state.range(0);

  Stack<Order> stack;
  Order output{};

  for (auto _ : state) {
    for (std::uint64_t i = 0; i < num_elements; i++) {
      stack.push(Order{.id = i, .side = Side::Buy, .type = OrderType::Limit, .quantity = i});
    }

    for (std::uint64_t i = 0; i < num_elements; i++) {
      bool result = stack.pop(output);
      benchmark::DoNotOptimize(result);
      benchmark::DoNotOptimize(output);
    }
  }
}

static void BM_real_traffic(benchmark::State& state) {
  const auto num_elements = state.range(0);

  std::mt19937 generator{42};
  std::bernoulli_distribution choose_push{0.5};

  std::vector<Events> events;
  events.reserve(num_elements);

  std::size_t depth = 0;

  for (std::size_t i = 0; i < num_elements; ++i) {
    if (depth == 0 || choose_push(generator)) {
      events.push_back(Events::Push);
      ++depth;
    } else {
      events.push_back(Events::Pop);
      --depth;
    }
  }

  while (depth > 0) {
    events.push_back(Events::Pop);
    --depth;
  }

  Order output{};
  Stack<Order> stack;

  for (auto _ : state) {
    std::uint64_t input = 0;

    for (auto event : events) {
      if (event == Events::Push) {
        stack.push(
            Order{.id = input++, .side = Side::Buy, .type = OrderType::Limit, .quantity = 10});

        benchmark::DoNotOptimize(stack);
      } else {
        bool result = stack.pop(output);
        benchmark::DoNotOptimize(result);
        benchmark::DoNotOptimize(output);
      }
    }

    benchmark::ClobberMemory();
  }

  state.SetItemsProcessed(state.iterations() *
                          static_cast<benchmark::IterationCount>(events.size()));
}

BENCHMARK(BM_push_pop);
BENCHMARK(BM_peek);
BENCHMARK(BM_fill_then_empty)->Arg(100)->Arg(1000)->Arg(10000);
BENCHMARK(BM_real_traffic)->Arg(100)->Arg(1000)->Arg(10000);
