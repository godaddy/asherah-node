#ifndef HINTS_H
#define HINTS_H

#define unlikely(expr) __builtin_expect(!!(expr), 0)
#define likely(expr) __builtin_expect(!!(expr), 1)

#endif
