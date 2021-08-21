#ifndef IMAGE_H
#define IMAGE_H

#include <stdint.h>

/* images are texture data in 
an interchange format such as
BMP, PNG, TIF, etc. To be used
in the engine it will transformed
to a texture which can be used by
a shader, has mips, filtering, etc.
Loaded images transformed into 
textures are stored in a global
texture state to minimise duplicates,
track users, and free unused
textures. */

/* texture */
typedef enum {
	TEXTURE_2D,
	TEXTURE_CUBE,
	TEXTURE_ARRAY,
	TEXTURE_VOLUME
} texture_type;

typedef struct {
	char* data;
	uint32_t users = 0;
	bool unused;
} texture_t;

static struct {
	texture_t** textures;

} texture_state;

/* image */





#endif