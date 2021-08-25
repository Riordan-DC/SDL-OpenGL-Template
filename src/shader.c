#include "shader.h"

int shader_graphics_new(struct shader_t* shader, char* shader_vertex_source, int shader_vertex_len, char* shader_fragment_source, int shader_fragment_len) {
	const char* version = "#version 460\n";

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
	glBindAttribLocation(program, SHADER_DRAW_ID, "aDrawID");
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
	//lovrShaderSetupUniforms(shader);

	return SHADER_GRAPHICS;
}

enum shader_type shader_compute_new(struct shader_t* shader, char* shader_compute_source, int64_t shader_compute_len) {
	return SHADER_ERROR;
}

//enum shader_default_type shader_default(shader_t* shader, default_shader_type type, char** flags, uint32_t flags_count);


void shader_set_uniform(struct shader_t* shader, char* uniform_name, void* data, uint32_t start, uint32_t count, uint32_t size) {

}

void shader_delete(struct shader_t* shader) {

}
