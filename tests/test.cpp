#include "stack.hpp"
#include <gtest/gtest.h>
#include <optional>

TEST(stack_tests, push_multiple_values) {
  Stack<unsigned int> stack;

  stack.push(42);
  EXPECT_EQ(stack.peek(), 42);

  stack.push(45);
  EXPECT_EQ(stack.peek(), 45);

  stack.push(37);
  EXPECT_EQ(stack.peek(), 37);
}

TEST(stack_tests, pop_multiple_values) {
  Stack<unsigned int> stack;

  stack.push(10);
  stack.push(20);
  stack.push(30);

  unsigned int output = {};

  EXPECT_TRUE(stack.pop(output));
  EXPECT_EQ(output, 30);
  EXPECT_TRUE(stack.pop(output));
  EXPECT_EQ(output, 20);
  EXPECT_TRUE(stack.pop(output));
  EXPECT_EQ(output, 10);
}

TEST(stack_tests, pop_from_empty) {
  Stack<unsigned int> stack;

  unsigned int output = {};

  EXPECT_FALSE(stack.pop(output));
  EXPECT_EQ(output, 0);
}

TEST(stack_tests, ensure_capacity_works) {
  Stack<unsigned int> stack;

  unsigned int output = {};

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

TEST(stack_tests, ensure_peek_works) {
  Stack<unsigned int> stack;

  EXPECT_EQ(stack.peek(), std::nullopt);
}
