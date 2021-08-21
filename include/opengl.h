#ifndef OPENGL_H
#define OPENGL_H

#include <stdio.h>
#include <stdlib.h>

#include "glad.h"

/* opengl gpu calls
and a gpu_state struct
which manages the global
state of the gpu */

static struct {
	uint32_t program;

	// stats
	uint32_t shader_switches = 0;
	uint32_t shaders = 0;
} gpu_state;

static struct {
	void* alloc = NULL;
	void* free = NULL;
} allocator;

void gl_use_program(uint32_t program);
GLuint gl_compile_shader(GLenum type, const char** sources, int* lengths, int count);

void gl_allocator_set(void* alloc, void* free);
static void* gl_allocate(size_t size);
static void gl_free(void* ref);

#endif