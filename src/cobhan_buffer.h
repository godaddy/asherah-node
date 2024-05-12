#ifndef COBHAN_BUFFER_H
#define COBHAN_BUFFER_H

#include <cstdint>   // for int32_t
#include <cstring>   // for std::memcpy
#include <iostream>  // for std::terminate
#include <limits>    // for std::numeric_limits
#include <sstream>   // for std::ostringstream
#include <stdexcept> // for std::runtime_error, std::invalid_argument
#include <string>    // for std::string

class CobhanBuffer {
public:
  // Used for requesting a new heap-based buffer allocation that can handle
  // data_len_bytes of data
  explicit CobhanBuffer(size_t data_len_bytes) {
    if (data_len_bytes > max_int32_size) {
      throw std::invalid_argument(
          "Requested data length exceeds maximum allowable size");
    }
    allocation_size = DataSizeToAllocationSize(data_len_bytes);
    cbuffer = new char[allocation_size];
    ownership = true;
    initialize(data_len_bytes);
  }

  // Used for passing a stack-based buffer allocation that hasn't been
  // initialized yet
  explicit CobhanBuffer(char *cbuffer, size_t allocation_size)
      : cbuffer(cbuffer), allocation_size(allocation_size), ownership(false) {
    if (allocation_size > max_int32_size) {
      throw std::invalid_argument(
          "Allocation size exceeds maximum allowable size");
    }
    initialize();
  }

  // Move constructor
  CobhanBuffer(CobhanBuffer &&other) noexcept { moveFrom(std::move(other)); }

  // Move assignment operator
  CobhanBuffer &operator=(CobhanBuffer &&other) noexcept {
    if (this != &other) {
      cleanup();
      moveFrom(std::move(other));
    }
    return *this;
  }

  // Implicit cast to void*
  operator void *() const { // NOLINT(*-explicit-constructor)
    return static_cast<void *>(cbuffer);
  }

  char *get_data_ptr() const { return data_ptr; }

  size_t get_data_len_bytes() const { return *data_len_ptr; }

  ~CobhanBuffer() {
    verify_canaries();
    cleanup();
  }

    __attribute__((unused)) std::string DebugPrintStdErr() const {
    auto debug_string = std::string(get_data_ptr(), get_data_len_bytes());
    std::ostringstream debug_output;
    debug_output << "CobhanBuffer { " << std::endl
                 << "data_len_bytes=" << get_data_len_bytes() << std::endl
                 << ", allocation_size=" << get_allocation_size() << std::endl
                 << ", data_ptr=" << static_cast<void *>(get_data_ptr())
                 << std::endl
                 << ", canary1=" << static_cast<void *>(canary1_ptr)
                 << std::endl
                 << ", canary2=" << static_cast<void *>(canary2_ptr)
                 << std::endl
                 << ", ownership=" << ownership << std::endl
                 << ", string=[" << debug_string << "]" << std::endl
                 << ", string_length=" << debug_string.length() << std::endl
                 << "}" << std::endl
                 << std::flush;
    return debug_output.str();
  }

  static size_t DataSizeToAllocationSize(size_t data_len_bytes) {
    size_t allocation =
        data_len_bytes + cobhan_header_size_bytes +
        canary_size_bytes       // Add space for canary value
        + safety_padding_bytes; // Add safety padding if configured
    if (allocation > max_int32_size) {
      throw std::invalid_argument(
          "Calculated allocation size exceeds maximum allowable size");
    }
    return allocation;
  }

  static size_t AllocationSizeToMaxDataSize(size_t allocation_len_bytes) {
    size_t data_len_bytes = allocation_len_bytes - cobhan_header_size_bytes -
                            canary_size_bytes - safety_padding_bytes;
    if (data_len_bytes > max_int32_size) {
      throw std::invalid_argument(
          "Calculated data size exceeds maximum allowable size");
    }
    return data_len_bytes;
  }

protected:
  void verify_canaries() const {
    if (*canary1_ptr != 0) {
      std::cerr << "Canary 1 corrupted! Expected: 0, Found: " << *canary1_ptr
                << std::endl
                << std::flush;
      std::cerr << "CobhanBuffer: Memory corruption detected: Canary values "
                   "are corrupted.  Terminating process."
                << std::endl
                << std::flush;
      std::terminate();
    }
    if (*canary2_ptr != canary_constant) {
      std::cerr << "Canary 2 corrupted! Expected: 0xdeadbeef, Found: "
                << *canary2_ptr << std::endl
                << std::flush;
      std::cerr << "CobhanBuffer: Memory corruption detected: Canary values "
                   "are corrupted.  Terminating process."
                << std::endl
                << std::flush;
      std::terminate();
    }
  }

  size_t get_allocation_size() const { return allocation_size; }

  void set_data_len_bytes(size_t data_len_bytes) {
    if (data_len_bytes > max_int32_size) {
      throw std::invalid_argument(
          "Requested data length exceeds maximum allowable size");
    }
    if (data_len_bytes > allocation_size) {
      throw std::invalid_argument(
          "Requested data length exceeds allocation size");
    }
    *data_len_ptr = static_cast<int32_t>(data_len_bytes);
  }

private:
  // Assumes cbuffer and allocation_size are already set
  void initialize(size_t data_len_bytes = 0) {
    // Save pointer to data
    data_ptr = cbuffer + cobhan_header_size_bytes;
    // First int32_t is the data length
    data_len_ptr = reinterpret_cast<int32_t *>(cbuffer);
    // Second int32_t is reserved for future use
    auto reserved_ptr = reinterpret_cast<int32_t *>(cbuffer + sizeof(int32_t));
    // First canary value is an int32_t 0 which gives us four NULLs
    canary1_ptr = reinterpret_cast<int32_t *>(cbuffer + allocation_size -
                                              canary_size_bytes);
    // Second canary value is an int32_t 0xdeadbeef
    canary2_ptr = canary1_ptr + 1;

    // Calculate the data length
    if (data_len_bytes == 0) {
      data_len_bytes = AllocationSizeToMaxDataSize(allocation_size);
    }

    if (data_len_bytes > max_int32_size) {
      throw std::invalid_argument("Data length exceeds maximum allowable size");
    }

    // Write Cobhan header values
    *data_len_ptr = static_cast<int32_t>(data_len_bytes);

    // Reserved for future use
    *reserved_ptr = 0;

    // Write canary values
    *canary1_ptr = 0;
    *canary2_ptr = canary_constant;
  }

  void moveFrom(CobhanBuffer &&other) {
    other.verify_canaries();

    if (other.ownership) {
      // Transfer ownership of the existing buffer
      cbuffer = other.cbuffer;
      allocation_size = other.allocation_size;
      ownership = true;
      data_ptr = other.data_ptr;
      data_len_ptr = other.data_len_ptr;
      canary1_ptr = other.canary1_ptr;
      canary2_ptr = other.canary2_ptr;

      // Reset the other object to prevent it from deallocating the buffer
      other.cbuffer = nullptr;
      other.allocation_size = 0;
      other.ownership = false;
    } else {
      // Allocate a new buffer and copy the contents
      allocation_size = other.allocation_size;
      if (allocation_size > max_int32_size) {
        throw std::invalid_argument(
            "Allocation size exceeds maximum allowable size");
      }

      cbuffer = new char[allocation_size];
      std::memcpy(cbuffer, other.cbuffer, allocation_size);
      ownership = true;
      initialize(*other.data_len_ptr);
    }
  }

  void cleanup() {
    if (ownership) {
      delete[] cbuffer;
    }
    cbuffer = nullptr;
    allocation_size = 0;
  }

  char *cbuffer = nullptr;
  size_t allocation_size = 0;
  bool ownership = false;
  int32_t *data_len_ptr = nullptr;
  char *data_ptr = nullptr;
  int32_t *canary1_ptr = nullptr;
  int32_t *canary2_ptr = nullptr;

  static constexpr int32_t canary_constant = static_cast<int32_t>(0xdeadbeef);
  static constexpr size_t cobhan_header_size_bytes =
      sizeof(int32_t) * 2; // 2x int32_t headers
  static constexpr size_t canary_size_bytes =
      sizeof(int32_t) * 2; // Two int32_t canaries
  static constexpr size_t safety_padding_bytes = 8;
  static constexpr size_t max_int32_size =
      static_cast<size_t>(std::numeric_limits<int32_t>::max());
};

#endif // COBHAN_BUFFER_H
