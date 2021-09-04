#include <stdarg.h>
#include <stddef.h>
#include <stdint.h>

/*
* LOVR utils by bjornbytes (Bjorn Swenson) @ 
* https://github.com/bjornbytes/lovr
* LICENSE: MIT
*/

#ifndef UTIL_H
#define UTIL_H

#ifdef _WIN32
#define ROY_NORETURN __declspec(noreturn)
#define ROY_THREAD_LOCAL __declspec(thread)
#define ROY_RESTRICT __restrict
#else
#define ROY_NORETURN __attribute__((noreturn))
#define ROY_THREAD_LOCAL __thread
#define ROY_RESTRICT restrict
#endif

#ifndef M_PI
#define M_PI 3.14159265358979
#endif
#define TAU 2.0*M_PI

#define MAX(a, b) (a > b ? a : b)
#define MIN(a, b) (a < b ? a : b)
#define CLAMP(x, min, max) MAX(min, MIN(max, x))
#define ALIGN(p, n) (((uintptr_t) (p) + (n - 1)) & ~(n - 1))
#define CHECK_SIZEOF(T) int(*_o)[sizeof(T)]=1
#define RAD(x) x * (float)M_PI / 180.0
#define DEG(x) x * 180.0 / (float)M_PI

#define BIT_VALUE(x, bit) (x >> bit) & 1
#define SET_BIT(x, bit) x |= (1 << bit)
#define CLEAR_BIT(x, bit) x &= ~(1 << bit)
#define TOGGLE_BIT(x, bit) x ^= (1 << bit)
#define IS_BIT_SET(x, bit) (BIT_VALUE(x,bit) == 1)

typedef struct color_t { float r, g, b, a; } color_t;

#ifdef __cplusplus
extern "C" {
#endif

// Error handling
typedef void error_fn(void*, const char*, va_list);
extern ROY_THREAD_LOCAL error_fn* lovrErrorCallback;
extern ROY_THREAD_LOCAL void* lovrErrorUserdata;
void roy_set_error_callback(error_fn* callback, void* userdata);
void ROY_NORETURN roy_throw(const char* format, ...);
#define roy_assert(c, ...) if (!(c)) { roy_throw(__VA_ARGS__); }

// Logging
typedef void log_fn(void*, int, const char*, const char*, va_list);
enum { LOG_DEBUG, LOG_INFO, LOG_WARN, LOG_ERROR };
void roy_set_log_callback(log_fn* callback, void* userdata);
void roy_log(int level, const char* tag, const char* format, ...);

// Hash function (FNV1a)
static inline uint64_t hash64(const void* data, size_t length) {
  const uint8_t* bytes = (const uint8_t*) data;
  uint64_t hash = 0xcbf29ce484222325;
  for (size_t i = 0; i < length; i++) {
    hash = (hash ^ bytes[i]) * 0x100000001b3;
  }
  return hash;
}

// Dynamic Array
typedef void* arr_allocator(void* data, size_t size);
#define arr_t(T) struct { T* data; arr_allocator* alloc; size_t length, capacity; }
#define arr_init(a, allocator) (a)->data = NULL, (a)->length = 0, (a)->capacity = 0, (a)->alloc = allocator
#define arr_free(a) if ((a)->data) (a)->alloc((a)->data, 0)
#define arr_reserve(a, n) _arr_reserve((void**) &((a)->data), n, &(a)->capacity, sizeof(*(a)->data), (a)->alloc)
#define arr_expand(a, n) arr_reserve(a, (a)->length + n)
#define arr_push(a, x) arr_reserve(a, (a)->length + 1), (a)->data[(a)->length++] = x
#define arr_pop(a) (a)->data[--(a)->length]
#define arr_append(a, p, n) arr_reserve(a, (a)->length + n), memcpy((a)->data + (a)->length, p, n * sizeof(*(p))), (a)->length += n
#define arr_splice(a, i, n) memmove((a)->data + (i), (a)->data + ((i) + n), ((a)->length - (i) - (n)) * sizeof(*(a)->data)), (a)->length -= n
#define arr_clear(a) (a)->length = 0

static inline void _arr_reserve(void** data, size_t n, size_t* capacity, size_t stride, arr_allocator* allocator) {
  if (*capacity >= n) return;
  if (*capacity == 0) *capacity = 1;
  while (*capacity < n) *capacity *= 2;
  *data = allocator(*data, *capacity * stride);
  roy_assert(*data, "Array out of memory");
}

// UTF-8
size_t utf8_decode(const char *s, const char *e, unsigned *pch);
void utf8_encode(uint32_t codepoint, char str[4]);

#ifdef __cplusplus
}
#endif

#endif

