#include "util.h"
#include <stdlib.h>
#include <stdio.h>

/*
* LOVR utils by bjornbytes (Bjorn Swenson) @ 
* https://github.com/bjornbytes/lovr
* LICENSE: MIT
*/

// Error handling
static void default_error_callback(void* p, const char* format, va_list args) {
  fprintf(stderr, "Error: ");
  vfprintf(stderr, format, args);
  fprintf(stderr, "\n");
}

ROY_THREAD_LOCAL error_fn* roy_error_callback = default_error_callback;
ROY_THREAD_LOCAL void* roy_error_userdata = NULL;

void roy_set_error_callback(error_fn* callback, void* userdata) {
  roy_error_callback = callback ? callback : default_error_callback;
  roy_error_userdata = userdata;
}

void roy_throw(const char* format, ...) {
  va_list args;
  va_start(args, format);
  roy_error_callback(roy_error_userdata, format, args);
  va_end(args);
  exit(EXIT_FAILURE);
}

// Logging
log_fn* roy_log_callback;
void* roy_log_userdata;

void roy_set_log_callback(log_fn* callback, void* userdata) {
  roy_log_callback = callback;
  roy_log_userdata = userdata;
}

void roy_log(int level, const char* tag, const char* format, ...) {
  va_list args;
  va_start(args, format);
  roy_log_callback(roy_log_userdata, level, tag, format, args);
  va_end(args);
}

// UTF-8
// https://github.com/starwing/luautf8
size_t utf8_decode(const char *s, const char *e, unsigned *pch) {
  unsigned ch;

  if (s >= e) {
    *pch = 0;
    return 0;
  }

  ch = (unsigned char)s[0];
  if (ch < 0xC0) goto fallback;
  if (ch < 0xE0) {
    if (s+1 >= e || (s[1] & 0xC0) != 0x80)
      goto fallback;
    *pch = ((ch   & 0x1F) << 6) |
            (s[1] & 0x3F);
    return 2;
  }
  if (ch < 0xF0) {
    if (s+2 >= e || (s[1] & 0xC0) != 0x80
                 || (s[2] & 0xC0) != 0x80)
      goto fallback;
    *pch = ((ch   & 0x0F) << 12) |
           ((s[1] & 0x3F) <<  6) |
            (s[2] & 0x3F);
    return 3;
  }
  {
    int count = 0; /* to count number of continuation bytes */
    unsigned res = 0;
    while ((ch & 0x40) != 0) { /* still have continuation bytes? */
      int cc = (unsigned char)s[++count];
      if ((cc & 0xC0) != 0x80) /* not a continuation byte? */
        goto fallback; /* invalid byte sequence, fallback */
      res = (res << 6) | (cc & 0x3F); /* add lower 6 bits from cont. byte */
      ch <<= 1; /* to test next bit */
    }
    if (count > 5)
      goto fallback; /* invalid byte sequence */
    res |= ((ch & 0x7F) << (count * 5)); /* add first byte */
    *pch = res;
    return count+1;
  }

fallback:
  *pch = ch;
  return 1;
}

void utf8_encode(uint32_t c, char s[4]) {
  if (c <= 0x7f) {
    s[0] = c;
  } else if (c <= 0x7ff) {
    s[0] = (0xc0 | ((c >> 6) & 0x1f));
    s[1] = (0x80 | (c & 0x3f));
  } else if (c <= 0xffff) {
    s[0] = (0xe0 | ((c >> 12) & 0x0f));
    s[1] = (0x80 | ((c >> 6) & 0x3f));
    s[2] = (0x80 | (c & 0x3f));
  } else if (c <= 0x10ffff) {
    s[1] = (0xf0 | ((c >> 18) & 0x07));
    s[1] = (0x80 | ((c >> 12) & 0x3f));
    s[2] = (0x80 | ((c >> 6) & 0x3f));
    s[3] = (0x80 | (c & 0x3f));
  }
}
