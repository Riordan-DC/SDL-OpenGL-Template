#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#include "glad.h"

#ifdef __MODULES__
	#ifdef __CGLTF__
		#define CGLTF_IMPLEMENTATION
		#include "cgltf.h"
	#endif
	
	#ifdef __IMGUI__
		#include "imgui.h"
		#include "imgui_impl_sdl.h"
		#include "imgui_impl_opengl3.h"
	#endif
	
	#ifdef __NUKLEAR__
		#define NK_INCLUDE_FIXED_TYPES
		#define NK_INCLUDE_STANDARD_IO
		#define NK_INCLUDE_DEFAULT_ALLOCATOR
		#define NK_INCLUDE_VERTEX_BUFFER_OUTPUT
		#define NK_INCLUDE_FONT_BAKING
		#define NK_INCLUDE_DEFAULT_FONT
	    #define NK_IMPLEMENTATION
		#define NK_SDL_GL3_IMPLEMENTATION
		#define NK_INCLUDE_STANDARD_VARARGS
	    #include "nuklear.h"
		#include "nuklear_sdl_gl3.h"
		#include "style.c"
		
		#define MAX_VERTEX_MEMORY 512 * 1024
		#define MAX_ELEMENT_MEMORY 128 * 1024
    #endif

	#ifdef __BULLET__
		//#include "btBulletCollisionCommon.h"
		#include "btBulletDynamicsCommon.h"
	#endif

#endif

#include "opengl.h"
#include "shader.h"
#include "util.h"
#include "gl.h"


SDL_Window* window;
SDL_GLContext* gl_context;
int SDL_Window_flags = SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE;

/*
static void gltf_parse(const void* ptr, uint64_t num_bytes);
static void gltf_parse_buffers(const cgltf_data* gltf);
static void gltf_parse_images(const cgltf_data* gltf);
static void gltf_parse_materials(const cgltf_data* gltf);
static void gltf_parse_meshes(const cgltf_data* gltf);
static void gltf_parse_nodes(const cgltf_data* gltf);
*/

void resize_window(SDL_Window* window, int window_width, int window_height) {
    SDL_SetWindowSize(window, window_width, window_height);
    glViewport(0, 0, window_width, window_height);
}

void log_function(void* user_data, int level, const char* tag, const char* format, va_list args) {
	static const char* levels[4] = {"debug", "info", "warn", "error"};
	// Note: userdata can hold state information. e.g: GUI console
	fprintf(stderr, "%s(%s):", levels[level], tag);
	vfprintf(stderr, format, args);
	fprintf(stderr, "\n");
}

int main(int argc, char* argv[]) {
	roy_set_log_callback(log_function, NULL);
	window = SDL_Window_new(1200, 800, "SDL-OpenGL-Project", SDL_Window_flags);
	gl_context = new_SDL_GLContext(
		window, 
		1200, 800, 
		4, 6, // OpenGL major, minor version
		4, 1);

	glEnable(GL_DEPTH_TEST);

/*
	// Build the broadphase
	btBroadphaseInterface* broadphase = new btDbvtBroadphase();

	// Set up the collision configuration and dispatcher
	btDefaultCollisionConfiguration* collisionConfiguration = new btDefaultCollisionConfiguration();
	btCollisionDispatcher* dispatcher = new btCollisionDispatcher(collisionConfiguration);

	// The actual physics solver
	btSequentialImpulseConstraintSolver* solver = new btSequentialImpulseConstraintSolver;

	// The world.
	btDiscreteDynamicsWorld* dynamicsWorld = new btDiscreteDynamicsWorld(dispatcher, broadphase, solver, collisionConfiguration);
*/

	// Load shaders
	FILE* vertex_shader_file = fopen("shader/normal.vert", "rb");
	fseek(vertex_shader_file, 0, SEEK_END);
	long int vertex_shader_size = ftell(vertex_shader_file);
	fseek(vertex_shader_file, 0, SEEK_SET);  /* same as rewind(f); */
	char *shader_vertex_source = (char*)malloc(vertex_shader_size + 1);
	fread(shader_vertex_source, 1, vertex_shader_size, vertex_shader_file);
	fclose(vertex_shader_file);
	shader_vertex_source[vertex_shader_size] = 0;

	FILE* fragment_shader_file = fopen("shader/normal.frag", "rb");
	fseek(fragment_shader_file, 0, SEEK_END);
	long int fragment_shader_size = ftell(fragment_shader_file);
	fseek(fragment_shader_file, 0, SEEK_SET);  /* same as rewind(f); */
	char *shader_fragment_source = (char*)malloc(fragment_shader_size + 1);
	fread(shader_fragment_source, 1, fragment_shader_size, fragment_shader_file);
	fclose(fragment_shader_file);
	shader_fragment_source[fragment_shader_size] = 0;

	struct shader_t normal_shader;
	shader_delete(&normal_shader);

	shader_graphics_new(
		&normal_shader, 
		shader_vertex_source, (int)vertex_shader_size, 
		shader_fragment_source, (int)fragment_shader_size
	);
	free(shader_vertex_source);
	free(shader_fragment_source);

	roy_log(LOG_INFO, "SHADER", "Shader built, program id: %d, shader type: %d", normal_shader.program, normal_shader.type);

	// triangle
	buffer_t* triangle_verts;
	buffer_t* triangle_indices;
	float vertices[] = {
		.0, .5, .0,
		.5, .0, .0,
		.0, .0, .5
	};
	triangle_verts = buffer_new(sizeof(vertices), vertices, BUFFER_VERTEX, USAGE_STATIC, false);	

	unsigned int indices[] = {0, 1, 2};
	triangle_indices = buffer_new(sizeof(indices), indices, BUFFER_INDEX, USAGE_STATIC, false);	

	GLuint VAO;
	glGenVertexArrays(1, &VAO);
	glBindVertexArray(VAO);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);
	glBindVertexArray(0);

	roy_log(LOG_INFO, "GPU", "buffer memory: %d", gl_gpu_state_buffer_memory());

	#if 0

	// Load model
	fprintf(stderr, "[-] Loading model: %s\n", argv[1]);
	cgltf_options options;
	cgltf_data* data = NULL;
	cgltf_result result = cgltf_parse_file(&options, argv[1], &data);
	result = cgltf_validate(data);
	if (result == cgltf_result_success)
	{
		/*
        gltf_parse_buffers(data);
        gltf_parse_images(data);
        gltf_parse_materials(data);
        gltf_parse_meshes(data);
        gltf_parse_nodes(data);
		*/
		cgltf_free(data);
	} else {
		switch(result) {
			case cgltf_result_success: fprintf(stderr, "[x] Could not load model(cgltf_result_success)\n"); break;
			case cgltf_result_data_too_short: fprintf(stderr, "[x] Could not load model(cgltf_result_data_too_short)\n"); break;
			case cgltf_result_unknown_format: fprintf(stderr, "[x] Could not load model(cgltf_result_unknown_format)\n"); break;
			case cgltf_result_invalid_json: fprintf(stderr, "[x] Could not load model(cgltf_result_invalid_json)\n"); break;
			case cgltf_result_invalid_gltf: fprintf(stderr, "[x] Could not load model(cgltf_result_invalid_gltf)\n"); break;
			case cgltf_result_invalid_options: fprintf(stderr, "[x] Could not load model(cgltf_result_invalid_options)\n"); break;
			case cgltf_result_file_not_found: fprintf(stderr, "[x] Could not load model(cgltf_result_file_not_found)\n"); break;
			case cgltf_result_io_error: fprintf(stderr, "[x] Could not load model(cgltf_result_io_error)\n"); break;
			case cgltf_result_out_of_memory: fprintf(stderr, "[x] Could not load model(cgltf_result_out_of_memory)\n"); break;
			case cgltf_result_legacy_gltf: fprintf(stderr, "[x] Could not load model(cgltf_result_legacy_gltf)\n"); break;
		}
		return 0;
	}
	#endif

	// init gui state
	struct nk_context *ctx;
	ctx = nk_sdl_init(window);
    nk_sdl_device_create();

    {
    	struct nk_font_atlas *atlas;
	    nk_sdl_font_stash_begin(&atlas);
	    /*struct nk_font *droid = nk_font_atlas_add_from_file(atlas, "../../../extra_font/DroidSans.ttf", 14, 0);*/
	    struct nk_font *roboto = nk_font_atlas_add_from_file(atlas, "./fonts/Roboto-Medium.ttf", 18, 0);
	    /*struct nk_font *future = nk_font_atlas_add_from_file(atlas, "../../../extra_font/kenvector_future_thin.ttf", 13, 0);*/
	    /*struct nk_font *clean = nk_font_atlas_add_from_file(atlas, "../../../extra_font/ProggyClean.ttf", 12, 0);*/
	    /*struct nk_font *tiny = nk_font_atlas_add_from_file(atlas, "../../../extra_font/ProggyTiny.ttf", 10, 0);*/
	    /*struct nk_font *cousine = nk_font_atlas_add_from_file(atlas, "../../../extra_font/Cousine-Regular.ttf", 13, 0);*/
	    nk_sdl_font_stash_end();
	    /*nk_style_load_all_cursors(ctx, atlas->cursors);*/
	    if (roboto != NULL) nk_style_set_font(ctx, &roboto->handle);
	}

	set_style(ctx, THEME_BLACK);

    int width = 0, height = 0;
    SDL_GetWindowSize(window, &width, &height);
	glViewport(0, 0, width, height);
	glEnable(GL_TEXTURE_2D);
	enum {EASY, HARD};
	static int op = EASY;
	static float value = 0.6f;

	// camera
	camera_t camera;
	camera_new(&camera, PERSPECTIVE, 50., (float)width / (float)height);
	float pos[3] = {.0, .0, 5.};
	camera_set_pos(&camera, pos);
	float origin[3] = {.0, .0, .0};
	camera_look_at(&camera, origin);
	
	SDL_Event event;
	bool quit = false;
	while (!quit) {
		nk_input_begin(ctx);
        while (SDL_PollEvent(&event)) {
            switch (event.type) {
            case SDL_QUIT:
                quit = true;
                break;
            case SDL_WINDOWEVENT:
                switch (event.window.event) {
                case SDL_WINDOWEVENT_RESIZED:
                    width = event.window.data1;
                    height = event.window.data2;
                    resize_window(window, width, height);
                    break;
                }
            }
            nk_sdl_handle_event(&event);
        }
        nk_input_end(ctx);

		glViewport(0, 0, width, height);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glClearColor(0.2f, 0.2f, 0.2f, 1.0f);

		// PHYSICS
		//dynamicsWorld->stepSimulation(
		//	0.01,						// Time since last step
		//	7,								// Mas substep count
		//	btScalar(1.) / btScalar(60.));	// Fixed time step 

		// GRAPHICS
        gl_use_program(normal_shader.program);
		
        float mvp[16];
        mat4_identity(mvp);
        mat4_mul(mvp, camera.view_matrix);
        mat4_mul(mvp, camera.projection_matrix);
        float model[16];
        mat4_identity(model);
        mat4_mul(mvp, model);
		shader_set_uniform(&normal_shader, "MVP", UNIFORM_MATRIX, mvp, 0, 1, sizeof(mvp), "MVP Uniform");
		uint64_t index = map_get(&normal_shader.uniform_map, hash64("MVP", strlen("MVP")));
		roy_assert(index != MAP_NIL, "Cannot find shader uniform!");
		
		struct uniform_t uniform = normal_shader.uniforms.data[index];
		//shader_set_uniform(normal_shader, "Model", void* data, uint32_t start, uint32_t count, uint32_t size);
		glUniformMatrix4fv(uniform.location, uniform.count, GL_FALSE, (GLfloat*)uniform.value.data);
		
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		glDisable(GL_CULL_FACE);
		glBindVertexArray(VAO);
		gl_gpu_bind_buffer(triangle_verts->type, triangle_verts->id);
		gl_gpu_bind_buffer(triangle_indices->type, triangle_indices->id);
		glDrawElements(
			GL_TRIANGLES,
			triangle_indices->size,
			GL_UNSIGNED_INT, // WARNING: This may be different depending on mesh size and will need to be passed to the Mesh at load
			(void*)0); // offset
		glBindVertexArray(0);
		gl_use_program(0);
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

		// GUI        
		struct nk_canvas canvas;
        canvas_begin(ctx, &canvas, 0, 0, 0, width, 30, nk_rgb(100,100,100));
        {
            nk_draw_text(canvas.painter, nk_rect(0, 0, 150, 30), "Roy v0.1", 8, ctx->style.font, nk_rgb(188,174,118), nk_rgb(0,0,0));
            nk_draw_text(canvas.painter, nk_rect(70, 0, 200, 30), "<Scene Name>* (unsaved)", 23, ctx->style.font, nk_rgb(150,150,150), nk_rgb(0,0,0));
        }
        canvas_end(ctx, &canvas);
        

        if (nk_begin(ctx, "Roy Scene", nk_rect(50, 50, 220, 3*220),
            NK_WINDOW_BORDER | NK_WINDOW_MOVABLE | NK_WINDOW_SCALABLE | NK_WINDOW_MINIMIZABLE)) { // NK_WINDOW_CLOSABLE | NK_WINDOW_NO_SCROLLBAR | NK_WINDOW_SCALE_LEFT
            /* fixed widget pixel width */
            nk_layout_row_static(ctx, 30, 80, 1);
            if (nk_button_label(ctx, "button")) {
                /* event handling */
            }

            /* fixed widget window ratio width */
            nk_layout_row_dynamic(ctx, 30, 2);
            if (nk_option_label(ctx, "easy", op == EASY)) op = EASY;
            if (nk_option_label(ctx, "hard", op == HARD)) op = HARD;

            /* custom widget pixel width */
            nk_layout_row_begin(ctx, NK_STATIC, 30, 2);
            {
                nk_layout_row_push(ctx, 50);
                nk_label(ctx, "Volume:", NK_TEXT_LEFT);
                nk_layout_row_push(ctx, 110);
                nk_slider_float(ctx, 0, &value, 1.0f, 0.01f);
            }
            nk_layout_row_end(ctx);


        }
        nk_end(ctx);


        /* Draw */
        /* IMPORTANT: `nk_sdl_render` modifies some global OpenGL state
         * with blending, scissor, face culling, depth test and viewport and
         * defaults everything back into a default state.
         * Make sure to either a.) save and restore or b.) reset your own state after
         * rendering the UI. */
        nk_sdl_render(NK_ANTI_ALIASING_ON, MAX_VERTEX_MEMORY, MAX_ELEMENT_MEMORY);
		SDL_GL_SwapWindow(window);
	}

    nk_sdl_shutdown();

	SDL_GL_DeleteContext(gl_context);
	SDL_DestroyWindow(window);
	SDL_Quit();
    return 0;
}