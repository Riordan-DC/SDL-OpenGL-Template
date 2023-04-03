#include "shader.h"

enum shader_type shader_graphics_new(struct shader_t* shader, char* shader_vertex_source, int shader_vertex_len, char* shader_fragment_source, int shader_fragment_len) {
	const char* version = "#version 410\n";

	const char* compute_ext = "";

	const char* flag_src = ""; // todo:

	// Vertex
	const char* vertex_sources[] = { version, compute_ext, flag_src ? flag_src : "", shader_vertex_source };
	int32_t vertex_src_lens[] = { -1, -1, -1, shader_vertex_len};
	int vertex_source_count = sizeof(vertex_sources) / sizeof(vertex_sources[0]);
	GLuint vertex_shader = gl_compile_shader(GL_VERTEX_SHADER, vertex_sources, vertex_src_lens, vertex_source_count);

	// Fragment
	const char* fragment_sources[] = { version, compute_ext, flag_src ? flag_src : "", shader_fragment_source };
	int fragment_src_lens[] = { -1, -1, -1, shader_fragment_len};
	int fragment_source_count = sizeof(fragment_sources) / sizeof(fragment_sources[0]);
	GLuint fragment_shader = gl_compile_shader(GL_FRAGMENT_SHADER, fragment_sources, fragment_src_lens, fragment_source_count);

	// Link
	uint32_t program = glCreateProgram();
	glAttachShader(program, vertex_shader);
	glAttachShader(program, fragment_shader);
	glBindAttribLocation(program, SHADER_POSITION, "aPosition");
	glBindAttribLocation(program, SHADER_NORMAL, "aNormal");
	glBindAttribLocation(program, SHADER_TEX_COORD, "aTexCoord");
	glBindAttribLocation(program, SHADER_VERTEX_COLOR, "aVertexColor");
	glBindAttribLocation(program, SHADER_TANGENT, "aTangent");
	glBindAttribLocation(program, SHADER_BONES, "aBones");
	glBindAttribLocation(program, SHADER_BONE_WEIGHTS, "aBoneWeights");
	glBindAttribLocation(program, SHADER_DRAW_ID, "aDrawID"); // GPU Instance ID
	program = gl_link_program(program);
	glDetachShader(program, vertex_shader);
	glDeleteShader(vertex_shader);
	glDetachShader(program, fragment_shader);
	glDeleteShader(fragment_shader);
	shader->program = program;
	shader->type = SHADER_GRAPHICS;

	// Generic attributes
	gl_use_program(program);
	glVertexAttrib4fv(SHADER_VERTEX_COLOR, (float[4]) { 1., 1., 1., 1. });
	glVertexAttribI4uiv(SHADER_BONES, (uint32_t[4]) { 0., 0., 0., 0. });
	glVertexAttrib4fv(SHADER_BONE_WEIGHTS, (float[4]) { 1., 0., 0., 0. });
	glVertexAttribI4ui(SHADER_DRAW_ID, 0, 0, 0, 0);
	shader_setup_uniforms(shader);

    // Attribute cache
	int32_t attributeCount;
	glGetProgramiv(program, GL_ACTIVE_ATTRIBUTES, &attributeCount);
	map_init(&shader->attributes, attributeCount);
	for (int i = 0; i < attributeCount; i++) {
		char name[MAX_ATTRIBUTE_LENGTH];
		GLint size;
		GLenum type;
		GLsizei length;
		glGetActiveAttrib(program, i, MAX_ATTRIBUTE_LENGTH, &length, &size, &type, name);
		int location = glGetAttribLocation(program, name);
		if (location >= 0) {
			map_set(&shader->attributes, hash64(name, length), (location << 1) | is_attribute_type_integer(type));
		}
	}

	return SHADER_GRAPHICS;
}

enum shader_type shader_compute_new(struct shader_t* shader, char* shader_compute_source, int shader_compute_len) {
	return SHADER_ERROR;
}

//enum shader_default_type shader_default(shader_t* shader, default_shader_type type, char** flags, uint32_t flags_count);

void shader_set_uniform(struct shader_t* shader, char* uniform_name, enum shader_uniform_type type, void* data, uint32_t start, uint32_t count, uint32_t size, const char* debug) {
	/* updates the shader struct uniform, cpu-side. LOVR works by setting a uniform dirty bit which
	 * triggers a gpu uniform update. This mechansism saves redundant data streaming between the cpu
	 * and gpu. */
	uint64_t index = map_get(&shader->uniform_map, hash64(uniform_name, strlen(uniform_name)));
	if (index == MAP_NIL) {
		return;
	}

	struct uniform_t uniform = shader->uniforms.data[index];
	roy_assert(uniform.type == type, "Unable to send %ss to uniform %s", debug, uniform_name);
	roy_assert((start + count) * size <= uniform.size, "Too many %ss for uniform %s, maximum is %d", debug, uniform_name, uniform.size / size);

	void* dest = uniform.value.bytes + start * size;
	if (memcmp(dest, data, count * size)) {
		//lovrGraphicsFlushShader(shader);
		memcpy(dest, data, count * size);
		uniform.dirty = true;
	}
}

void shader_delete(struct shader_t* shader) {

}

static enum shader_uniform_type get_uniform_type(GLenum type, const char* debug) {
    switch (type) {
    case GL_FLOAT:
    case GL_FLOAT_VEC2:
    case GL_FLOAT_VEC3:
    case GL_FLOAT_VEC4:
        return UNIFORM_FLOAT;
    case GL_INT:
    case GL_INT_VEC2:
    case GL_INT_VEC3:
    case GL_INT_VEC4:
        return UNIFORM_INT;
    case GL_FLOAT_MAT2:
    case GL_FLOAT_MAT3:
    case GL_FLOAT_MAT4:
        return UNIFORM_MATRIX;
    case GL_SAMPLER_2D:
    case GL_SAMPLER_3D:
    case GL_SAMPLER_CUBE:
    case GL_SAMPLER_2D_ARRAY:
    case GL_SAMPLER_2D_SHADOW:
        return UNIFORM_SAMPLER;
#ifdef GL_ARB_shader_image_load_store
    case GL_IMAGE_2D:
    case GL_IMAGE_3D:
    case GL_IMAGE_CUBE:
    case GL_IMAGE_2D_ARRAY:
        return UNIFORM_IMAGE;
#endif
    default:
        roy_throw("Unsupported uniform type for uniform '%s'", debug);
        return UNIFORM_FLOAT;
    }
}

static int get_uniform_components(GLenum type) {
    switch (type) {
    case GL_FLOAT_VEC2: case GL_INT_VEC2: case GL_FLOAT_MAT2: return 2;
    case GL_FLOAT_VEC3: case GL_INT_VEC3: case GL_FLOAT_MAT3: return 3;
    case GL_FLOAT_VEC4: case GL_INT_VEC4: case GL_FLOAT_MAT4: return 4;
    default: return 1;
    }
}

static texture_type get_uniform_texture_type(GLenum type) {
    switch (type) {
    case GL_SAMPLER_2D: return TEXTURE_2D;
    case GL_SAMPLER_3D: return TEXTURE_VOLUME;
    case GL_SAMPLER_CUBE: return TEXTURE_CUBE;
    case GL_SAMPLER_2D_ARRAY: return TEXTURE_ARRAY;
    case GL_SAMPLER_2D_SHADOW: return TEXTURE_2D;
#ifdef GL_ARB_shader_image_load_store
    case GL_IMAGE_2D: return TEXTURE_2D;
    case GL_IMAGE_3D: return TEXTURE_VOLUME;
    case GL_IMAGE_CUBE: return TEXTURE_CUBE;
    case GL_IMAGE_2D_ARRAY: return TEXTURE_ARRAY;
#endif
    default: return TEXTURE_UNKNOWN;
    }
}

static void shader_setup_uniforms(struct shader_t* shader) {
    uint32_t program = shader->program;
    gl_use_program(program); // TODO necessary?

    // Uniform blocks
    int32_t blockCount;
    glGetProgramiv(program, GL_ACTIVE_UNIFORM_BLOCKS, &blockCount);
    roy_assert((size_t)blockCount <= MAX_BLOCK_BUFFERS, "Shader has too many uniform blocks (%d) the max is %d", blockCount, MAX_BLOCK_BUFFERS);
    map_init(&shader->blockMap, blockCount);

    arr_block_t* uniformBlocks = &shader->blocks[BLOCK_UNIFORM];
    arr_init(uniformBlocks, realloc);
    arr_reserve(uniformBlocks, (size_t)blockCount);
    for (int i = 0; i < blockCount; i++) {
        struct uniform_block_t block = { .slot = i, .source = NULL };
        glUniformBlockBinding(program, i, block.slot);

        GLsizei length;
        char name[MAX_UNIFORM_LENGTH];
        glGetActiveUniformBlockName(program, i, MAX_UNIFORM_LENGTH, &length, name);
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
    map_init(&shader->uniform_map, 0);
    arr_init(&shader->uniforms, realloc);
    for (uint32_t i = 0; i < (uint32_t)uniformCount; i++) {
        struct uniform_t uniform;
        GLenum glType;
        GLsizei length;
        glGetActiveUniform(program, i, MAX_UNIFORM_LENGTH, &length, &uniform.count, &glType, uniform.name);
        roy_log(LOG_INFO, "SHADER", "Setup shader uniform:%s", uniform.name);

        char* subscript = strchr(uniform.name, '[');
        if (subscript) {
            if (subscript[1] > '0') {
                continue;
            }
            else {
                *subscript = '\0';
                length = subscript - uniform.name;
            }
        }

        uniform.location = glGetUniformLocation(program, uniform.name);
        uniform.type = get_uniform_type(glType, uniform.name);
        uniform.components = get_uniform_components(glType);
        uniform.shadow = glType == GL_SAMPLER_2D_SHADOW;
        uniform.image = glType == GL_IMAGE_2D || glType == GL_IMAGE_3D || glType == GL_IMAGE_CUBE || glType == GL_IMAGE_2D_ARRAY;
        uniform.textureType = get_uniform_texture_type(glType);
        uniform.baseSlot = uniform.type == UNIFORM_SAMPLER ? textureSlot : (uniform.type == UNIFORM_IMAGE ? imageSlot : -1);
        uniform.dirty = false;

        int blockIndex;
        glGetActiveUniformsiv(program, 1, &i, GL_UNIFORM_BLOCK_INDEX, &blockIndex);

        if (blockIndex != -1) {
            struct uniform_block_t* block = &shader->blocks[BLOCK_UNIFORM].data[blockIndex];
            glGetActiveUniformsiv(program, 1, &i, GL_UNIFORM_OFFSET, &uniform.offset);
            glGetActiveUniformsiv(program, 1, &i, GL_UNIFORM_SIZE, &uniform.count);
            if (uniform.count > 1) {
                int stride;
                glGetActiveUniformsiv(program, 1, &i, GL_UNIFORM_ARRAY_STRIDE, &stride);
                uniform.size = stride * uniform.count;
            }
            else if (uniform.type == UNIFORM_MATRIX) {
                int matrixStride;
                glGetActiveUniformsiv(program, 1, &i, GL_UNIFORM_MATRIX_STRIDE, &matrixStride);
                uniform.size = uniform.components * matrixStride;
            }
            else {
                uniform.size = 4 * (uniform.components == 3 ? 4 : uniform.components);
            }

            arr_push(&block->uniforms, uniform);
            continue;
        }
        else if (uniform.location == -1) {
            continue;
        }

        switch (uniform.type) {
        case UNIFORM_FLOAT:
            uniform.size = uniform.components * uniform.count * sizeof(float);
            uniform.value.data = calloc(1, uniform.size);
            roy_assert(uniform.value.data, "Out of memory");
            break;

        case UNIFORM_INT:
            uniform.size = uniform.components * uniform.count * sizeof(int);
            uniform.value.data = calloc(1, uniform.size);
            roy_assert(uniform.value.data, "Out of memory");
            break;

        case UNIFORM_MATRIX:
            uniform.size = uniform.components * uniform.components * uniform.count * sizeof(float);
            uniform.value.data = calloc(1, uniform.size);
            roy_assert(uniform.value.data, "Out of memory");
            break;

        case UNIFORM_SAMPLER:
        case UNIFORM_IMAGE:
            uniform.size = uniform.count * sizeof(texture_t*); // (uniform.type == UNIFORM_SAMPLER ? sizeof(texture_t*) : sizeof(StorageImage));
            uniform.value.data = calloc(1, uniform.size);
            roy_assert(uniform.value.data, "Out of memory");

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
                char name[76 /* MAX_UNIFORM_LENGTH + 2 + 10 */];
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

        map_set(&shader->uniform_map, hash64(uniform.name, length), shader->uniforms.length);
        arr_push(&shader->uniforms, uniform);
        textureSlot += uniform.type == UNIFORM_SAMPLER ? uniform.count : 0;
        imageSlot += uniform.type == UNIFORM_IMAGE ? uniform.count : 0;
    }
}
