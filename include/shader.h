#ifndef SHADER_H
#define SHADER_H

#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>

#include "opengl.h"
#include "texture.h"
#include "util.h"
#include "map.h"

/* shader struct needs to track
a shader program on the gpu.*/

#define MAX_UNIFORM_NAME_LEN 64
#define MAX_UNIFORM_LENGTH 64
#define MAX_UNIFORMS 8
#define MAX_ATTRIBUTE_LENGTH 64
#define MAX_BLOCK_BUFFERS 8

#define SHADER_POSITION 0
#define SHADER_NORMAL 1
#define SHADER_TEX_COORD 2
#define SHADER_VERTEX_COLOR 3
#define SHADER_TANGENT 4
#define SHADER_BONES 5
#define SHADER_BONE_WEIGHTS 6
#define SHADER_DRAW_ID 7

enum shader_default_type {
	SHADER_UNLIT,
	SHADER_STANDARD,
	SHADER_CELL,
	SHADER_FONT,
	SHADER_DEFAULT_TYPES
};

enum shader_uniform_type {
	UNIFORM_ERROR,
	UNIFORM_FLOAT,
	UNIFORM_MATRIX,
	UNIFORM_INT,
	UNIFORM_SAMPLER,
	UNIFORM_IMAGE,
	UNIFORM_TYPES
};

enum shader_type {
	SHADER_ERROR,
	SHADER_GRAPHICS,
	SHADER_COMPUTE
};

enum shader_uniform_access {
	ACCESS_READ,
	ACCESS_WRITE,
	ACCESS_READ_WRITE
};

enum block_type {
	BLOCK_UNIFORM,
	BLOCK_COMPUTE
};

struct uniform_t {
	char name[MAX_UNIFORM_NAME_LEN];
	enum shader_uniform_type type;
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
};

typedef arr_t(struct uniform_t) arr_uniform_t;

struct uniform_block_t {
	arr_uniform_t uniforms;
	enum shader_uniform_access access;
	struct buffer_t* source;
	size_t offset;
	size_t size;
	int slot;
};

typedef arr_t(struct uniform_block_t) arr_block_t;

struct shader_t {
	uint32_t program;
	enum shader_type type;
	uint32_t ref;
	arr_uniform_t uniforms;
	arr_block_t blocks[2];
	map_t attributes;
	map_t uniform_map;
	map_t blockMap;
};



#ifdef __cplusplus
extern "C" {
#endif

int shader_graphics_new(struct shader_t* shader, char* shader_vertex_source, int shader_vertex_len, char* shader_fragment_source, int shader_fragment_len);
int shader_compute_new(struct shader_t* shader, char* shader_compute_source, int shader_compute_len);
//enum shader_type shader_default(shader_t* shader, default_shader_type type, char** flags, uint32_t flags_count);

static void shader_setup_uniforms(struct shader_t* shader);
void shader_set_uniform(struct shader_t* shader, char* uniform_name, enum shader_uniform_type type, void* data, uint32_t start, uint32_t count, uint32_t size, const char* debug);
void shader_delete(struct shader_t* shader);

#ifdef __cplusplus
}
#endif
/* Material is a struct that manages
a predefined vertex, fragment, and 
optionally a geometry shaders and
their uniforms. basic materials
should be: BPR and Cell*/

struct material_t {
	struct shader_t* shader;
};

#endif