#ifndef LOGGING_H
#define LOGGING_H
#include "hints.h"
#include <cstdint>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <string>

extern int32_t verbose_flag;

__attribute__((always_inline)) inline void debug_log(const char *function_name,
                                                     const char *message) {
  if (unlikely(verbose_flag)) {
    std::cerr << "asherah-node: [DEBUG] " << function_name << ": " << message
              << std::endl
              << std::flush;
  }
}

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
                                                     const char *message) {
  if (unlikely(verbose_flag)) {
    std::cerr << "asherah-node: [ERROR] " << function_name << ": " << message
              << std::endl
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

__attribute__((always_inline)) inline std::string format_ptr(char *ptr) {
  std::ostringstream ss;
  ss << "0x" << std::hex << (intptr_t)ptr;
  return ss.str();
}

__attribute__((always_inline, noreturn)) inline void
log_error_and_throw(const char *function_name, std::string error_msg) {
  error_log(function_name, error_msg);
  throw new std::runtime_error(function_name + (": " + error_msg));
}

#endif
