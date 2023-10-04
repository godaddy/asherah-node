#ifndef LOGGING_H
#define LOGGING_H
#include <cstdint>
#include <string>
#include "hints.h"

extern int32_t verbose_flag;

__attribute__((always_inline)) inline void debug_log(const char *function_name,
                                                     std::string message) {
  if (unlikely(verbose_flag)) {
    std::cerr << "asherah-node: [DEBUG] " << function_name << ": " << message
              << std::endl
              << std::flush;
  }
}

__attribute__((always_inline)) inline void
debug_log_alloca(const char *function_name, const char *variable_name,
                 size_t length) {
  if (unlikely(verbose_flag)) {
    std::cerr << "asherah-node: [DEBUG] " << function_name
              << ": Calling alloca(" << length << ") (stack) for "
              << variable_name << std::endl
              << std::flush;
  }
}

__attribute__((always_inline)) inline void error_log(const char *function_name,
                                                     std::string message) {
  if (unlikely(verbose_flag)) {
    std::cerr << "asherah-node: [ERROR] " << function_name << ": " << message
              << std::endl
              << std::flush;
  }
}

#endif
