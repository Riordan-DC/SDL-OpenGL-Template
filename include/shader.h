#ifndef SHADER_H
#define SHADER_H

#include <stdint.h>
#include <stdlib.h>

#include "opengl.h"
#include "texture.h"

/* shader struct needs to track
a shader program on the gpu.*/

#define MAX_UNIFORM_NAME_LEN 64
#define MAX_UNIFORMS 8

typedef struct {
	const char* shader_src;
	uint32_t shader_src_len;
	const char** flags;
	uint32_t flags_count;
} shader_src_t;

typedef enum {
	SHADER_UNLIT,
	SHADER_STANDARD,
	SHADER_CELL,
	SHADER_FONT,
	MAX_DEFAULT_SHADERS
} default_shader_type;

typedef enum {
	UNIFORM_FLOAT,
	UNIFORM_MATRIX,
	UNIFORM_INT,
	UNIFORM_SAMPLER,
	UNIFORM_IMAGE
} uniform_type;

typedef enum {
	SHADER_GRAPHICS,
	SHADER_COMPUTE
} shader_type;

typedef struct {
	char name[MAX_UNIFORM_NAME_LEN];
	uniform_type type;
	uint32_t components;
	uint32_t count;
	uint32_t location;
	uint32_t offset;
	uint32_t size;
	union {
		void* data;
		char* bytes;
		uint32_t* ints;
		float* floats;
		texture_t** textures;
		//StorageImage* images;
	} value;
	texture_type textureType;
	uint32_t baseSlot;
	bool shadow;
	bool image;
	bool dirty;
} uniform_t;

typedef struct {
	int32_t program;
	uniform_t uniforms[MAX_UNIFORMS];
	shader_type type;
} shader_t;

static struct {
	void* alloc = NULL;
	void* free = NULL;
} allocator;


shader_t* shader_new(shader_src_t* vert_src, shader_src_t* frag_src);
shader_t* shader_default(default_shader_type type, const char** flags, uint32_t flags_count);
shader_t* shader_compute(shader_src_t* comp_src);
void shader_set_uniform(shader_t* shader, char* uniform_name, void* data, uint32_t start, uint32_t count, uint32_t size);
void shader_delete(shader_t* shader);
shader_src_t* shader_src_new(const char* shader_src, uint32_t shader_src_len, const char** flags, uint32_t flags_count);

void shader_allocator_set(void* alloc, void* free);
static void* shader_allocate(size_t size);
static void shader_free(void* ref);

/* Material is a struct that manages
a predefined vertex, fragment, and 
optionally a geometry shaders and
their uniforms. basic materials
should be: BPR and Cell*/

struct material_t {

};

#endif