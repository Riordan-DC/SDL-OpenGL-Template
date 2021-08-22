#include "opengl.h"


void gl_use_program(uint32_t program) {
  if (gpu_state.program != program) {
    gpu_state.program = program;
    glUseProgram(program);
    gpu_state.shader_switches++;
  }
}

GLuint gl_compile_shader(GLenum type, const char** sources, int* lengths, int count) {
  GLuint shader = glCreateShader(type);
  glShaderSource(shader, count, sources, lengths);
  glCompileShader(shader);

  int isShaderCompiled;
  glGetShaderiv(shader, GL_COMPILE_STATUS, &isShaderCompiled);
  if (!isShaderCompiled) {
    int logLength;
    glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &logLength);
    char* log = (char*)gl_allocate(logLength);
    //lovrAssert(log, "Out of memory");
    glGetShaderInfoLog(shader, logLength, &logLength, log);
    const char* name;
    switch (type) {
      case GL_VERTEX_SHADER: name = "vertex shader"; break;
      case GL_FRAGMENT_SHADER: name = "fragment shader"; break;
      case GL_COMPUTE_SHADER: name = "compute shader"; break;
      default: name = "shader"; break;
    }
    fprintf(stderr, "Could not compile %s:\n%s", name, log);
  }

  return shader;
}

void gl_allocator_set(void* (*alloc)(size_t size), void (*free)(void* ref)) {
	gl_allocator.alloc = alloc;
	gl_allocator.free = free;
}

static void* gl_allocate(size_t size) {
	if (gl_allocator.alloc == NULL) {
		return malloc(size);
	} else {
		return gl_allocator.alloc(size);
	}
}

static void gl_free(void* ref) {
	if (gl_allocator.free == NULL) {
		free(ref);
	} else {
		gl_allocator.free(ref);
	}
}

