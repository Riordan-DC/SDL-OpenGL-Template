#include <stdint.h>

/*
* LOVR map by bjornbytes (Bjorn Swenson) @ 
* https://github.com/bjornbytes/lovr
* LICENSE: MIT
*/

#ifndef MAP_H
#define MAP_H

#define MAP_NIL UINT64_MAX

typedef struct {
  uint32_t size;
  uint32_t used;
  uint64_t* hashes;
  uint64_t* values;
} map_t;

#ifdef __cplusplus
extern "C" {
#endif

void map_init(map_t* map, uint32_t n);
void map_free(map_t* map);
uint64_t map_get(map_t* map, uint64_t hash);
void map_set(map_t* map, uint64_t hash, uint64_t value);
void map_remove(map_t* map, uint64_t hash);

#ifdef __cplusplus
}
#endif

#endif