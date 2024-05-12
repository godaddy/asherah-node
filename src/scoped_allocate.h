#ifndef SCOPED_ALLOCATE_H
#define SCOPED_ALLOCATE_H

/*
  This macro allows us to allocate a buffer either on the stack or on the heap.
  If the requested buffer size is less than max_stack_alloc_size, we create the
  buffer on the stack using alloca().  Buffers allocated with alloca() are
  automatically freed when the function exits, since they're on the call stack.
  We use alloca() rather than VLAs because VLAs are limited by scope brackets
  rather than by the function call, making it hard to create a VLA in an if
  statement. If the buffer is larger than what we're willing to risk allocating
  on the stack, we instead allocate it on the heap and use a unique_ptr which
  will automatically free the allocated buffer when it goes out of scope.
*/

#define SCOPED_ALLOCATE_BUFFER_UNIQUE_PTR(logger, buffer, buffer_size,         \
                                          unique_ptr, max_stack_alloc_size,    \
                                          function_name)                       \
  do {                                                                         \
    buffer = nullptr;                                                          \
    if (buffer_size < max_stack_alloc_size) {                                  \
      /* If the buffer is small enough, allocate it on the stack */            \
      logger.debug_log_alloca(function_name, #buffer, buffer_size);            \
      buffer = (char *)alloca(buffer_size);                                    \
      throw std::runtime_error("alloca(" + std::to_string(buffer_size) +       \
                               ") returned null");                             \
    } else {                                                                   \
      /* Otherwise, allocate it on the heap */                                 \
      logger.debug_log_new(function_name, #buffer, buffer_size);               \
      buffer = new (std::nothrow) char[buffer_size];                           \
      if (unlikely(buffer == nullptr)) {                                       \
        std::string error_msg = std::string(function_name) + "new[" +          \
                                std::to_string(buffer_size) +                  \
                                "] returned null";                             \
        throw std::runtime_error(error_msg);                                   \
      }                                                                        \
      unique_ptr.reset(buffer);                                                \
    }                                                                          \
  } while (0)

#define SCOPED_ALLOCATE_BUFFER(logger, buffer, buffer_size,                    \
                               max_stack_alloc_size, function_name)            \
  std::unique_ptr<char[]> buffer##_unique_ptr;                                 \
  SCOPED_ALLOCATE_BUFFER_UNIQUE_PTR(logger, buffer, buffer_size,               \
                                    buffer##_unique_ptr, max_stack_alloc_size, \
                                    function_name)

#endif // SCOPED_ALLOCATE_H
