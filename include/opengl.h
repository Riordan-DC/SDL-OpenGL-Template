/* 
opengl gpu calls
and a gpu_state struct
which manages the global
state of the gpu 
*/

#ifndef OPENGL_H
#define OPENGL_H

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <stdbool.h>

#include <SDL2/SDL.h>

#include "util.h"
#include "glad.h"

static struct {
	uint32_t program;

	// stats
	uint32_t shader_switches;
	uint32_t shaders;
	uint32_t buffer_count;
	size_t buffer_memory;
} gpu_state;

static struct {
	void* (*alloc)(size_t size);
	void (*free)(void* ref);
} gl_allocator;

#ifdef __cplusplus
extern "C" {
#endif

// OpenGL startup
void init_glad(void);

// GPU state

// GPU states
size_t gl_gpu_state_buffer_memory(void);

// Shaders
void gl_use_program(uint32_t program);
GLuint gl_compile_shader(GLenum type, const char** sources, int* lengths, int count);
GLuint gl_link_program(GLuint program);

void gl_allocator_set(void* (*alloc)(size_t size), void (*free)(void* ref));
//void gl_allocator_set(void* alloc, void* free);
static void* gl_allocate(size_t size);
static void gl_free(void* ref);

// OpenGL Buffers
typedef enum {
  BUFFER_VERTEX,
  BUFFER_INDEX,
  BUFFER_UNIFORM,
  BUFFER_SHADER_STORAGE,
  BUFFER_GENERIC,
  MAX_BUFFER_TYPES
} buffer_type;

typedef enum {
  USAGE_STATIC,
  USAGE_DYNAMIC,
  USAGE_STREAM
} buffer_usage;

struct buffer_t {
	buffer_type type;
	buffer_usage usage;
	uint32_t ref;
	uint32_t id;
	void* data;
	size_t size;
	size_t flush_from;
	size_t flush_to;
	bool mapped;
	bool readable;
	uint8_t incoherent;
};

struct buffer_t* buffer_new(size_t size, void* data, buffer_type type, buffer_usage usage, bool readable);
static GLenum convert_buffer_type(buffer_type type);
static GLenum convert_buffer_usage(buffer_usage usage);
void gl_gpu_bind_buffer(buffer_type type, uint32_t buffer);
void buffer_map();
void buffer_unmap();

bool is_attribute_type_integer(GLenum type);

#ifdef __cplusplus
}
#endif

#endif