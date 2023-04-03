#include "opengl.h"

void init_glad(void) {
  // Load OpenGL function pointers
  // This version of GLAD does NOT include a loader:
  //      gladLoadGL();
  roy_assert(gladLoadGLLoader((GLADloadproc)SDL_GL_GetProcAddress) != 0,  "Failed to initialize GLAD");

  glEnable(GL_CULL_FACE);
  glEnable(GL_DEPTH_TEST);
  glEnable(GL_MULTISAMPLE);
  roy_log(LOG_INFO, "OpenGL", "OpenGL Version [%s]", glGetString(GL_VERSION));
  roy_log(LOG_INFO, "CPU", "CPU Cache line size [%d bytes]", SDL_GetCPUCacheLineSize());
}

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
    roy_log(LOG_ERROR, "OpenGL", "Could not compile %s:\n%s", name, log);
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

GLuint gl_link_program(GLuint program) {
  glLinkProgram(program);

  int is_linked;
  glGetProgramiv(program, GL_LINK_STATUS, &is_linked);
  if (!is_linked) {
    int log_length;
    glGetProgramiv(program, GL_INFO_LOG_LENGTH, &log_length);
    char* log = (char*)malloc(log_length);
    roy_assert(log, "Out of memory");
    glGetProgramInfoLog(program, log_length, &log_length, log);
    roy_assert(0, "Could not link shader:\n%s", log);
  }

  return program;
}

// GPU states
size_t gl_gpu_state_buffer_memory(void) {
  return gpu_state.buffer_memory;
}

// OpenGL buffers
void gl_gpu_bind_buffer(buffer_type type, uint32_t buffer) {
  glBindBuffer(convert_buffer_type(type), buffer);
}

static GLenum convert_buffer_type(buffer_type type) {
  switch (type) {
    case BUFFER_VERTEX: return GL_ARRAY_BUFFER;
    case BUFFER_INDEX: return GL_ELEMENT_ARRAY_BUFFER;
    case BUFFER_UNIFORM: return GL_UNIFORM_BUFFER;
    case BUFFER_SHADER_STORAGE: return GL_SHADER_STORAGE_BUFFER;
    case BUFFER_GENERIC: return GL_COPY_WRITE_BUFFER;
    default: roy_throw("Unreachable");
  }
}

static GLenum convert_buffer_usage(buffer_usage usage) {
  switch (usage) {
    case USAGE_STATIC: return GL_STATIC_DRAW;
    case USAGE_DYNAMIC: return GL_DYNAMIC_DRAW;
    case USAGE_STREAM: return GL_STREAM_DRAW;
    default: roy_throw("Unreachable");
  }
}

struct buffer_t* buffer_new(size_t size, void* data, buffer_type type, buffer_usage usage, bool readable) {
  struct buffer_t *buffer = (struct buffer_t*)calloc(1, sizeof(struct buffer_t));
  roy_assert(buffer, "Out of memory");
  buffer->ref = 1;

  gpu_state.buffer_count++;
  gpu_state.buffer_memory += size;
  buffer->size = size;
  buffer->readable = readable;
  buffer->type = type;
  buffer->usage = usage;
  glGenBuffers(1, &buffer->id);
  gl_gpu_bind_buffer(type, buffer->id);
  GLenum glType = convert_buffer_type(type);

  buffer->data = malloc(size);
  roy_assert(buffer->data, "Out of memory");
  if (data) {
      memcpy(buffer->data, data, size);
  }
  glBufferData(glType, size, data, convert_buffer_usage(usage));

  // Testing
  GLint buf_size = 0;
  glGetBufferParameteriv(glType, GL_BUFFER_SIZE, &buf_size);
  if(size != buf_size)
  {
      glDeleteBuffers(1, &buffer->id);
      // Log the error
      assert(false); //, "OpenGL Error: Cannot buffer data.");
      return NULL;
  }
  return buffer;
}

bool is_attribute_type_integer(GLenum type) {
  switch (type) {
    case GL_INT:
    case GL_INT_VEC2:
    case GL_INT_VEC3:
    case GL_INT_VEC4:
    case GL_UNSIGNED_INT:
    case GL_UNSIGNED_INT_VEC2:
    case GL_UNSIGNED_INT_VEC3:
    case GL_UNSIGNED_INT_VEC4:
      return true;
    default:
      return false;
  }
}