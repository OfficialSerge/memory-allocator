#pragma once

#include <cstddef>
#include <memory>
#include <new>
#include <optional>
#include <stdexcept>
#include <type_traits>

class Arena {
public:
  /*
   * 'explicit' prevents implicit conversion from a size to an arena.
   *
   *  Arena arena{4096};   Valid: direct list initialization.
   *  Arena arena = 4096;  Invalid: copy initialization, cannot use.
   *
   * ::operator new() is a low-level allocation function that behaves like C's
   * malloc, it allocates raw uninitialized memory of a set 'byte_size' and
   * returns a void pointer. NOT like calling 'new', this just allocates bytes
   * it doesn't init anything.
   *
   * ::operator new() returns a void pointer, we static cast this to a std::byte
   * pointer since pointer arithmetic is guarenteed by the language standard to
   * be 1-byte.
   * */
  explicit Arena(std::size_t byte_size)
      : buffer_(static_cast<std::byte*>(::operator new(byte_size))), capacity_(byte_size),
        offset_(0) {}

  /*
   * ::operator delete() will release the buffer_ block of memory, it will not
   * however call destructors on all the contents of the buffer.
   * */
  ~Arena() { ::operator delete(buffer_); }

  void* allocate(std::size_t byte_size, std::size_t alignment) {
    // where we are now in the arena buffer
    std::byte* current_ptr = buffer_ + offset_;

    // how much space we have left in the buffer
    std::size_t space = capacity_ - offset_;

    // a pointer that std::align can adjust by referenece.
    void* aligned_ptr = current_ptr;

    /*
     * @param alignment - the desired alignment size (MUST BE A POWER OF 2)
     * @param size      - the number of bytes of storage to fit
     * @param ptr       - a reference to a pointer
     * @param space     - available bytes starting at ptr, passed by reference
     *
     * Inspects a buffer starting at 'ptr' with remaining capacity of 'space'
     * to see if there is enough room to accomodate 'size' bytes aligned to
     * 'alignment'.
     * */
    if (std::align(alignment, byte_size, aligned_ptr, space) == nullptr) {
      throw std::bad_alloc();
    }

    // new offset = aligned starting position + requested size
    offset_ = static_cast<std::byte*>(aligned_ptr) - buffer_ + byte_size;

    // return the aligned pointer, no object has been constructed here, just a
    // pointer to the aligned space.
    return aligned_ptr;
  }

  void* view(std::size_t byte_size) {
    /*
     * in the case where byte_size is greater than the size of the allocated
     * buffer, it will exceed the bounds of the arena, thing is it can only
     * be less than the lower bound since we subtract from offset_ and byte_size
     * is an unsigned int, so it can't be negative.
     * */
    if (byte_size > offset_) {
      return nullptr;
    }

    return buffer_ + (offset_ - byte_size);
  }

  // reset the offset into the area so we overwrite the contents with fresh data.
  void reset() { offset_ = 0; }

  void pop(std::size_t byte_size) {
    if (byte_size > offset_) {
      throw std::out_of_range("byte_size extends before arena storage.");
    }

    offset_ -= byte_size;
  }

  // no copy
  Arena(const Arena&) = delete;

  // no move
  Arena(Arena&&) = delete;

private:
  std::byte* buffer_;
  std::size_t capacity_;
  std::size_t offset_;
};

template <typename T>
  requires std::is_trivially_destructible_v<T>
class ArenaStack {
public:
  explicit ArenaStack(Arena* arena) noexcept : arena_(arena) {};

  // we don't manage our own memory, the arena will.
  ArenaStack(ArenaStack&&) = delete;
  ArenaStack(const ArenaStack&) = delete;
  ArenaStack& operator=(ArenaStack&&) = delete;
  ArenaStack& operator=(const ArenaStack&) = delete;
  ~ArenaStack() = default;

  void push(T value) {
    // use the arena to allocate the storage.
    void* storage = arena_->allocate(sizeof(T), alignof(T));

    // then initialize the struct T using our newly allocated space.
    ::new (storage) T{value};

    ++capacity_;
  }

  bool pop(T& output) {
    if (capacity_ == 0) {
      return false;
    }

    T* ptr = static_cast<T*>(arena_->view(sizeof(T)));
    if (ptr == nullptr) {
      return false;
    }

    output = *ptr;
    arena_->pop(sizeof(T));
    --capacity_;
    return true;
  }

  std::optional<T> peek() {
    if (capacity_ == 0) {
      return std::nullopt;
    }

    T* ptr = static_cast<T*>(arena_->view(sizeof(T)));
    if (ptr == nullptr) {
      return std::nullopt;
    }

    return *ptr;
  }

  unsigned int size() const noexcept { return capacity_; }

private:
  Arena* arena_;
  unsigned int capacity_{};
};
