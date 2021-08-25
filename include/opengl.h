#ifndef OPENGL_H
#define OPENGL_H

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#include <SDL2/SDL.h>

#include "util.h"
#include "glad.h"

/* opengl gpu calls
and a gpu_state struct
which manages the global
state of the gpu */

static struct {
	uint32_t program;

	// stats
	uint32_t shader_switches;
	uint32_t shaders;
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

// Shaders
void gl_use_program(uint32_t program);
GLuint gl_compile_shader(GLenum type, const char** sources, int* lengths, int count);
GLuint gl_link_program(GLuint program);

void gl_allocator_set(void* alloc, void* free);
static void* gl_allocate(size_t size);
static void gl_free(void* ref);

#ifdef __cplusplus
}
#endif

#endif