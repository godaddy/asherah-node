#ifndef LOGGING_H
#define LOGGING_H
#include <cstddef> // size_t
#include <cstdint> // int32_t
#include <sstream> // std::ostringstream
#include <string>  // std::string

class Logger {
public:
  void set_verbose_flag(int32_t verbose_flag);

  virtual void debug_log(const char *function_name,
                         const char *message) const = 0;
  virtual void debug_log(const char *function_name,
                         const std::string &message) const = 0;
  virtual void debug_log_alloca(const char *function_name,
                                const char *variable_name,
                                size_t length) const = 0;

  virtual void debug_log_new(const char *function_name,
                             const char *variable_name,
                             size_t length) const = 0;

  virtual void error_log(const char *function_name,
                         const char *message) const = 0;
  virtual void error_log(const char *function_name,
                         const std::string &message) const = 0;

  __attribute__((always_inline)) inline static std::string
  format_ptr(const void *ptr) {
    std::ostringstream ss;
    ss << "0x" << std::hex << (intptr_t)ptr;
    return ss.str();
  }

protected:
  bool verbose_flag = false;
  std::string system_name;

  explicit Logger(std::string system_name);
};

#endif // LOGGING_H
