#include "shader.h"

#define SHADER_POSITION 0
#define SHADER_NORMAL 1
#define SHADER_TEX_COORD 2
#define SHADER_VERTEX_COLOR 3
#define SHADER_TANGENT 4
#define SHADER_BONES 5
#define SHADER_BONE_WEIGHTS 6
#define SHADER_DRAW_ID 7

shader_t* shader_new(shader_src_t* vert_src, shader_src_t* frag_src) {
	shader_t* shader = (shader_t*)shader_allocate(sizeof(shader_t));

	const char* version = "#version 300\n";

	const char* compute_ext = "";

	const char* flag_src = ""; // todo:

	// Vertex
	const char* vertex_sources[] = { version, compute_ext, flag_src ? flag_src : "", vert_src->shader_src };
	int32_t vertex_src_lens[] = { -1, -1, -1, vert_src->shader_src_len};
	int vertexSourceCount = sizeof(vertex_sources) / sizeof(vertex_sources[0]);
	GLuint vertex_shader = gl_compile_shader(GL_VERTEX_SHADER, vertex_sources, vertex_src_lens, vertexSourceCount);

	// Fragment
	const char* fragment_sources[] = { version, compute_ext, flag_src ? flag_src : "", frag_src->shader_src };
	int fragment_src_lens[] = { -1, -1, -1, frag_src->shader_src_len};
	int fragmentSourceCount = sizeof(fragment_sources) / sizeof(fragment_sources[0]);
	GLuint fragment_shader = gl_compile_shader(GL_FRAGMENT_SHADER, fragment_sources, fragment_src_lens, fragmentSourceCount);

	shader_free(flag_src);

	// Link
	uint32_t program = glCreateProgram();
	glAttachShader(program, vertex_shader);
	glAttachShader(program, fragment_shader);
	glBindAttribLocation(program, SHADER_POSITION, "lovrPosition");
	glBindAttribLocation(program, SHADER_NORMAL, "lovrNormal");
	glBindAttribLocation(program, SHADER_TEX_COORD, "lovrTexCoord");
	glBindAttribLocation(program, SHADER_VERTEX_COLOR, "lovrVertexColor");
	glBindAttribLocation(program, SHADER_TANGENT, "lovrTangent");
	glBindAttribLocation(program, SHADER_BONES, "lovrBones");
	glBindAttribLocation(program, SHADER_BONE_WEIGHTS, "lovrBoneWeights");
	glBindAttribLocation(program, SHADER_DRAW_ID, "lovrDrawID");
	linkProgram(program);
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
	lovrShaderSetupUniforms(shader);

	return shader;
}

shader_t* shader_default(default_shader_t type, const char** flags, uint32_t flags_count) {
	shader_t* shader = shader_allocate(sizeof(shader_t));

	return shader;
}

shader_t* shader_compute(shader_src_t* comp_src) {
	shader_t* shader = shader_allocate(sizeof(shader_t));

	return shader;
}

void shader_set_uniform(shader_t* shader, char* uniform_name, void* data, uint32_t start, uint32_t count, uint32_t size) {

}

void shader_delete(shader_t* shader) {
	shader_free(shader); /* this does not free the uniform references */
}

shader_src_t* shader_src_new(const char* shader_src, uint32_t shader_src_len, const char** flags, uint32_t flags_count) {
	shader_src_t* shader_src = shader_allocate(sizeof(shader_src_t));

	shader_src->shader_src = shader_src;
	shader_src->shader_src_len = shader_src_len;
	shader_src->flags = flags;
	shader_src->flags_count = flags_count;

	return shader_src;
}

void shader_allocator_set(void* alloc, void* free) {
	allocator.alloc = alloc;
	allocator.free = free;
}

static void* shader_allocate(size_t size) {
	if (allocator.alloc == NULL) {
		return malloc(size);
	} else {
		return allocator.alloc(size);
	}
}

static void shader_free(void* ref) {
	if (allocator.free == NULL) {
		free(ref);
	} else {
		allocator.free(ref);
	}
}

