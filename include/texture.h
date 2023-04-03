#ifndef IMAGE_H
#define IMAGE_H

#include <stdint.h>
#include <stdbool.h>

#include "util.h"

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

/* image */
typedef enum {
	RGB8
} image_format;

struct image_t {
	unsigned char* data;
	unsigned int width, height;
	image_format format;
	unsigned short pixel_size_bytes;
};

void image_new(struct image_t* image);

void image_get_rgb(struct image_t* image, unsigned int x, unsigned int y, color_t* color) {
	uint8_t* pixel = (uint8_t*)image->data + ((y * image->width + x));
	color->r = pixel[0];
	color->g = pixel[1];
	color->b = pixel[2];
}

void image_get_rgba(struct image_t* image, unsigned int x, unsigned int y, color_t* color) {
	uint8_t* pixel = (uint8_t*)image->data + ((y * image->width + x));
	color->r = pixel[0];
	color->g = pixel[1];
	color->b = pixel[2];
	color->a = pixel[3];
}

/* texture */
typedef enum {
	TEXTURE_UNKNOWN = -1,
	TEXTURE_2D = 0,
	TEXTURE_CUBE = 1,
	TEXTURE_ARRAY = 2,
	TEXTURE_VOLUME = 3,

	// unused
	TEXTURE_1D, 
	TEXTURE_1D_ARRAY, 
	TEXTURE_2D_ARRAY, 
	TEXTURE_2D_MULTISAMPLE, 
	TEXTURE_2D_MULTISAMPLE_ARRAY, 
	TEXTURE_3D, 
	TEXTURE_CUBE_MAP, 
	TEXTURE_CUBE_MAP_ARRAY, 
	TEXTURE_RECTANGLE
} texture_type;

typedef enum {
	RED,
	RGB,
	RGBA,
	RG
} texture_format;

/*
typedef enum {

} texture_parameter;
*/

typedef struct {
	uint32_t id;
	char* data;
	uint32_t users;
	uint32_t properties;
	bool unused;
} texture_t;

void texture_new(texture_t* texture, struct image_t* image, uint32_t properties) {
	texture->properties = 0;
	texture->users = 0;
	texture->unused = true;

	glActiveTexture(GL_TEXTURE0);
	glGenTextures(1, &texture->id);
	glBindTexture(GL_TEXTURE_2D, texture->id);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

	// Upload to GPU Texture
	glTexImage2D(GL_TEXTURE_2D, 0, GL_R32F, image->width, image->height, 0, GL_RED, GL_FLOAT, image->data);

	// Generate mipmaps
	glGenerateMipmap(GL_TEXTURE_2D);

	glBindTexture(GL_TEXTURE_2D, 0);

	// Allocate GPU Texture space for partial derivative map
	//glActiveTexture(GL_TEXTURE1);
	//glGenTextures(1, &m_coarsemap.pdmap);
	//glBindTexture(GL_TEXTURE_2D, m_coarsemap.pdmap);
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST);
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	//glTexImage2D(GL_TEXTURE_2D, 0, GL_RG8, m_coarsemap_dim, m_coarsemap_dim, 0, GL_RG, GL_UNSIGNED_BYTE, NULL);
	//glGenerateMipmap(GL_TEXTURE_2D);
	//
	//glActiveTexture(GL_TEXTURE0);
}

static struct {
	texture_t** textures;

} texture_state;




#endif