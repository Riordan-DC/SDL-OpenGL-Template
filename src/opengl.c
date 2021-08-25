#include "opengl.h"

void init_glad(void) {
  // Load OpenGL function pointers
  // This version of GLAD does NOT include a loader:
  //      gladLoadGL();
  roy_assert(gladLoadGLLoader((GLADloadproc)SDL_GL_GetProcAddress) != 0,  "Failed to initialize GLAD");

  glEnable(GL_CULL_FACE);
  glEnable(GL_DEPTH_TEST);
  glEnable(GL_MULTISAMPLE);
  roy_log(LOG_INFO, "GL", "OpenGL Version [%s]\n", glGetString(GL_VERSION));
  roy_log(LOG_INFO, "GL", "CPU Cache line size [%d bytes]\n", SDL_GetCPUCacheLineSize());
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

GLuint gl_link_program(GLuint program) {
  glLinkProgram(program);

  int is_linked;
  glGetProgramiv(program, GL_LINK_STATUS, &is_linked);
  if (!is_linked) {
    int log_length;
    glGetProgramiv(program, GL_INFO_LOG_LENGTH, &log_length);
    char* log = malloc(log_length);
    roy_assert(log, "Out of memory");
    glGetProgramInfoLog(program, log_length, &log_length, log);
    roy_assert(0, "Could not link shader:\n%s", log);
  }

  return program;
}

#if 0
static void lovrShaderSetupUniforms(Shader* shader) {
  uint32_t program = shader->program;
  lovrGpuUseProgram(program); // TODO necessary?

  // Uniform blocks
  int32_t blockCount;
  glGetProgramiv(program, GL_ACTIVE_UNIFORM_BLOCKS, &blockCount);
  lovrAssert((size_t) blockCount <= MAX_BLOCK_BUFFERS, "Shader has too many uniform blocks (%d) the max is %d", blockCount, MAX_BLOCK_BUFFERS);
  map_init(&shader->blockMap, blockCount);
  
  arr_block_t* uniformBlocks = &shader->blocks[BLOCK_UNIFORM];
  arr_init(uniformBlocks, realloc);
  arr_reserve(uniformBlocks, (size_t) blockCount);
  for (int i = 0; i < blockCount; i++) {
    UniformBlock block = { .slot = i, .source = NULL };
    glUniformBlockBinding(program, i, block.slot);

    GLsizei length;
    char name[LOVR_MAX_UNIFORM_LENGTH];
    glGetActiveUniformBlockName(program, i, LOVR_MAX_UNIFORM_LENGTH, &length, name);
    int blockId = (i << 1) + BLOCK_UNIFORM;
    map_set(&shader->blockMap, hash64(name, length), blockId);
    arr_push(uniformBlocks, block);
    arr_init(&uniformBlocks->data[uniformBlocks->length - 1].uniforms, realloc);
  }

  // Shader storage buffers and their buffer variables
  arr_block_t* computeBlocks = &shader->blocks[BLOCK_COMPUTE];
  arr_init(computeBlocks, realloc);

  // Uniform introspection
  int32_t uniformCount;
  int textureSlot = 0;
  int imageSlot = 0;
  glGetProgramiv(program, GL_ACTIVE_UNIFORMS, &uniformCount);
  map_init(&shader->uniformMap, 0);
  arr_init(&shader->uniforms, realloc);
  for (uint32_t i = 0; i < (uint32_t) uniformCount; i++) {
    Uniform uniform;
    GLenum glType;
    GLsizei length;
    glGetActiveUniform(program, i, LOVR_MAX_UNIFORM_LENGTH, &length, &uniform.count, &glType, uniform.name);

    char* subscript = strchr(uniform.name, '[');
    if (subscript) {
      if (subscript[1] > '0') {
        continue;
      } else {
        *subscript = '\0';
        length = subscript - uniform.name;
      }
    }

    uniform.location = glGetUniformLocation(program, uniform.name);
    uniform.type = getUniformType(glType, uniform.name);
    uniform.components = getUniformComponents(glType);
    uniform.shadow = glType == GL_SAMPLER_2D_SHADOW;
    uniform.image = glType == GL_IMAGE_2D || glType == GL_IMAGE_3D || glType == GL_IMAGE_CUBE || glType == GL_IMAGE_2D_ARRAY;
    uniform.textureType = getUniformTextureType(glType);
    uniform.baseSlot = uniform.type == UNIFORM_SAMPLER ? textureSlot : (uniform.type == UNIFORM_IMAGE ? imageSlot : -1);
    uniform.dirty = false;

    int blockIndex;
    glGetActiveUniformsiv(program, 1, &i, GL_UNIFORM_BLOCK_INDEX, &blockIndex);

    if (blockIndex != -1) {
      UniformBlock* block = &shader->blocks[BLOCK_UNIFORM].data[blockIndex];
      glGetActiveUniformsiv(program, 1, &i, GL_UNIFORM_OFFSET, &uniform.offset);
      glGetActiveUniformsiv(program, 1, &i, GL_UNIFORM_SIZE, &uniform.count);
      if (uniform.count > 1) {
        int stride;
        glGetActiveUniformsiv(program, 1, &i, GL_UNIFORM_ARRAY_STRIDE, &stride);
        uniform.size = stride * uniform.count;
      } else if (uniform.type == UNIFORM_MATRIX) {
        int matrixStride;
        glGetActiveUniformsiv(program, 1, &i, GL_UNIFORM_MATRIX_STRIDE, &matrixStride);
        uniform.size = uniform.components * matrixStride;
      } else {
        uniform.size = 4 * (uniform.components == 3 ? 4 : uniform.components);
      }

      arr_push(&block->uniforms, uniform);
      continue;
    } else if (uniform.location == -1) {
      continue;
    }

    switch (uniform.type) {
      case UNIFORM_FLOAT:
        uniform.size = uniform.components * uniform.count * sizeof(float);
        uniform.value.data = calloc(1, uniform.size);
        lovrAssert(uniform.value.data, "Out of memory");
        break;

      case UNIFORM_INT:
        uniform.size = uniform.components * uniform.count * sizeof(int);
        uniform.value.data = calloc(1, uniform.size);
        lovrAssert(uniform.value.data, "Out of memory");
        break;

      case UNIFORM_MATRIX:
        uniform.size = uniform.components * uniform.components * uniform.count * sizeof(float);
        uniform.value.data = calloc(1, uniform.size);
        lovrAssert(uniform.value.data, "Out of memory");
        break;

      case UNIFORM_SAMPLER:
      case UNIFORM_IMAGE:
        uniform.size = uniform.count * (uniform.type == UNIFORM_SAMPLER ? sizeof(Texture*) : sizeof(StorageImage));
        uniform.value.data = calloc(1, uniform.size);
        lovrAssert(uniform.value.data, "Out of memory");

        // Use the value for ints to bind texture slots, but use the value for textures afterwards.
        for (int j = 0; j < uniform.count; j++) {
          uniform.value.ints[j] = uniform.baseSlot + j;
        }
        glUniform1iv(uniform.location, uniform.count, uniform.value.ints);
        memset(uniform.value.data, 0, uniform.size);
        break;
    }

    size_t offset = 0;
    for (int j = 0; j < uniform.count; j++) {
      int location = uniform.location;

      if (uniform.count > 1) {
        char name[76 /* LOVR_MAX_UNIFORM_LENGTH + 2 + 10 */];
        snprintf(name, sizeof(name), "%s[%d]", uniform.name, j);
        location = glGetUniformLocation(program, name);
      }

      switch (uniform.type) {
        case UNIFORM_FLOAT: glGetUniformfv(program, location, &uniform.value.floats[offset]); break;
        case UNIFORM_INT: glGetUniformiv(program, location, &uniform.value.ints[offset]); break;
        case UNIFORM_MATRIX: glGetUniformfv(program, location, &uniform.value.floats[offset]); break;
        default: break;
      }

      offset += uniform.components * (uniform.type == UNIFORM_MATRIX ? uniform.components : 1);
    }

    map_set(&shader->uniformMap, hash64(uniform.name, length), shader->uniforms.length);
    arr_push(&shader->uniforms, uniform);
    textureSlot += uniform.type == UNIFORM_SAMPLER ? uniform.count : 0;
    imageSlot += uniform.type == UNIFORM_IMAGE ? uniform.count : 0;
  }
}
#endif