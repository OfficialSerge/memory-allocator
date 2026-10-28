#pragma once

#include <optional>

template <typename T> class Stack {
private:
  struct Node {
    T value;
    Node* next = nullptr;

    ~Node() { delete next; }
  };

  Node* head = nullptr;
  unsigned int _capacity{};

public:
  Stack() = default;

  Stack(Stack&&) = delete;
  Stack(const Stack&) = delete;
  Stack& operator=(Stack&&) = delete;
  Stack& operator=(const Stack&) = delete;

  ~Stack() { delete head; };

  void push(T value) {
    Node* curr = new Node{value};

    curr->next = head;
    head = curr;

    ++_capacity;
  }

  bool pop(T& output) {
    if (head == nullptr) {
      return false;
    }

    output = head->value;

    Node* temp = head;
    head = head->next;
    temp->next = nullptr;
    delete temp;

    --_capacity;
    return true;
  }

  std::optional<T> peek() const {
    if (head == nullptr) {
      return std::nullopt;
    }

    return head->value;
  }

  unsigned int size() const noexcept { return _capacity; }
};
