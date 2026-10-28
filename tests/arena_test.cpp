#include "arena_stack.hpp"
#include <gtest/gtest.h>
#include <optional>

TEST(arena_stack_tests, push_multiple_values) {
  Arena stack_arena{sizeof(unsigned int) * 1024};
  ArenaStack<unsigned int> stack{&stack_arena};

  stack.push(42);
  EXPECT_EQ(stack.peek(), 42);

  stack.push(11);
  EXPECT_EQ(stack.peek(), 11);

  stack.push(10);
  EXPECT_EQ(stack.peek(), 10);
}

TEST(arena_stack_tests, pop_multiple_values) {
  Arena stack_arena{sizeof(unsigned int) * 1024};
  ArenaStack<unsigned int> stack{&stack_arena};

  stack.push(10);
  stack.push(20);
  stack.push(30);

  unsigned int output{};

  EXPECT_TRUE(stack.pop(output));
  EXPECT_EQ(output, 30);
  EXPECT_TRUE(stack.pop(output));
  EXPECT_EQ(output, 20);
  EXPECT_TRUE(stack.pop(output));
  EXPECT_EQ(output, 10);
}

TEST(arena_stack_tests, pop_from_empty) {
  Arena stack_arena{sizeof(unsigned int) * 1024};
  ArenaStack<unsigned int>stack{&stack_arena};

  unsigned int output{};
  EXPECT_FALSE(stack.pop(output));
  EXPECT_EQ(output, 0);
}

TEST(arena_stack_tests, test_capacity_works) {
  Arena stack_arena{sizeof(unsigned int) * 1024};
  ArenaStack<unsigned int>stack{&stack_arena};

  unsigned int output{};
  EXPECT_EQ(stack.size(), 0);

  stack.push(10);
  EXPECT_EQ(stack.size(), 1);

  stack.push(10);
  EXPECT_EQ(stack.size(), 2);

  stack.pop(output);
  EXPECT_EQ(stack.size(), 1);

  stack.pop(output);
  EXPECT_EQ(stack.size(), 0);

  EXPECT_FALSE(stack.pop(output));
  EXPECT_EQ(stack.size(), 0);
}

TEST(arena_stack_tests, ensure_peek_works){
  Arena stack_arena{sizeof(unsigned int) * 1024};
  ArenaStack<unsigned int> stack{&stack_arena};

  EXPECT_EQ(stack.peek(), std::nullopt);
}
