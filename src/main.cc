#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <float.h>
#include <math.h>

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
		#include "BulletCollision/CollisionShapes/btHeightfieldTerrainShape.h"
	#endif

	#ifdef __STB_IMAGE__
		#define STB_IMAGE_IMPLEMENTATION
		#define STBI_FAILURE_USERMSG 
		#include "stb_image.h"
	#endif
#endif

#include "opengl.h"
#include "shader.h"
#include "util.h"
#include "gl.h"
#include "editor.hpp"

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

void gui_transform(nk_context* ctx, const char* name, mat4 transform) {
	nk_layout_row_dynamic(ctx, 25, 1);
	nk_label(ctx, name, NK_TEXT_LEFT);
	nk_layout_row_dynamic(ctx, 25, 3);

	float position[4];
	mat4_getPosition(transform, position);
	float scale[4];
	mat4_getScale(transform, scale);
	// Remove scale to allow rotation to work
	mat4_scale(transform, 1./scale[0], 1./scale[1], 1./scale[2]);
	float yaw, pitch, roll;
	mat4_toEuler(transform, &yaw, &pitch, &roll);


	// Rotation
	// This took forever because the floating point error which accumulated every frame.
	float rot_mat_b[16];
	float oyaw=yaw, opitch=pitch, oroll=roll;
	yaw = DEG(yaw);
	pitch = DEG(pitch);
	roll = DEG(roll);

	float rotation_min = -180.0;
	float rotation_max = 180.0;
	nk_property_float(ctx, "RX", rotation_min, &yaw, rotation_max, .01f, 0.02f);
	nk_property_float(ctx, "RY", rotation_min, &pitch, rotation_max, .01f, 0.02f);
	nk_property_float(ctx, "RZ", rotation_min, &roll, rotation_max, .01f, 0.02f);
	yaw = RAD(yaw);
	pitch = RAD(pitch);
	roll = RAD(roll);

	yaw = fabsf(yaw - oyaw) > CMP_EPSILON ? yaw : oyaw;
	pitch = fabsf(pitch - opitch) > CMP_EPSILON ? pitch : opitch;
	roll = fabsf(roll - oroll) > CMP_EPSILON ? roll : oroll;
	mat4_fromEuler(rot_mat_b, yaw, pitch, roll);
	int equal = mat4_equalMat4(transform, rot_mat_b);
	if (equal) {
		// do nothing
	}
	else {
		mat4_init(transform, rot_mat_b);
	}

	// Position
	float position_min = -99999.0;
	float position_max = 99999.0;
	nk_property_float(ctx, "PX", position_min, &position[0], position_max, .01f, 0.02f);
	nk_property_float(ctx, "PY", position_min, &position[1], position_max, .01f, 0.02f);
	nk_property_float(ctx, "PZ", position_min, &position[2], position_max, .01f, 0.02f);
	transform[12] = position[0];
	transform[13] = position[1];
	transform[14] = position[2];

	// Scale (this may mess with rotation...)
	float scale_min = -99999.0;
	float scale_max = 99999.0;
	nk_property_float(ctx, "SX", scale_min, &scale[0], scale_max, .01f, 0.02f);
	nk_property_float(ctx, "SY", scale_min, &scale[1], scale_max, .01f, 0.02f);
	nk_property_float(ctx, "SZ", scale_min, &scale[2], scale_max, .01f, 0.02f);
	mat4_scale(transform, scale[0], scale[1], scale[2]);
}

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

void camera_handle_input(camera_t* camera, double dt) {
	// Rotate
	if (SDL_GetRelativeMouseMode() == SDL_TRUE) {
		int x, y;
		SDL_GetRelativeMouseState(&x, &y);

		float xoffset = (float)x;
		float yoffset = -(float)y;

		float sensitivity = 0.1f; // change this value to your liking
		xoffset *= sensitivity;
		yoffset *= sensitivity;

		camera->yaw += xoffset;
		camera->pitch += yoffset;

		// make sure that when pitch is out of bounds, screen doesn't get flipped
		if (camera->pitch > 89.0f)
			camera->pitch = 89.0f;
		if (camera->pitch < -89.0f)
			camera->pitch = -89.0f;
	}

	// Movement
	const Uint8 *state = SDL_GetKeyboardState(NULL);
	float speed = 1.0 * (float)dt;
	float pos[4] = {.0, .0, .0, 1.};
	float front[4], right[4], up[4];
	camera_basis(camera, front, right, up);
	if (state[SDL_SCANCODE_W]) { // forwards
		float move_dir[4] = { .0, .0, .0, 1. };
		vec3_init(move_dir, front);
		vec3_scale(move_dir, speed);
	    vec3_add(pos, move_dir);
	}
	if (state[SDL_SCANCODE_S]) { // backwards
		float move_dir[4] = { .0, .0, .0, 1. };
		vec3_init(move_dir, front);
		vec3_scale(move_dir, speed);
	    vec3_sub(pos, move_dir);
	}
	if (state[SDL_SCANCODE_A]) { // left
		float move_dir[4] = { .0, .0, .0, 1. };
		vec3_init(move_dir, front);
		vec3_cross(move_dir, up);
		vec3_normalize(move_dir);
		vec3_scale(move_dir, speed);
	    vec3_sub(pos, move_dir);
	}
	if (state[SDL_SCANCODE_D]) { // right
		float move_dir[4] = { .0, .0, .0, 1. };
		vec3_init(move_dir, front);
		vec3_cross(move_dir, up);
		vec3_normalize(move_dir);
		vec3_scale(move_dir, speed);
	    vec3_add(pos, move_dir);
	}
	vec3_add(camera->position, pos);
}

int main(int argc, char* argv[]) {
	roy_set_log_callback(log_function, NULL);
	window = SDL_Window_new(1200, 800, "SDL-OpenGL-Project", SDL_Window_flags);
	gl_context = new_SDL_GLContext(
		window, 
		1200, 800, 
		4, 6, // OpenGL major, minor version (4.6) (3.1 for bullet debug)
		4, 1);

	glEnable(GL_DEPTH_TEST);

	// Build the broadphase
	btBroadphaseInterface* broadphase = new btDbvtBroadphase();

	// Set up the collision configuration and dispatcher
	btDefaultCollisionConfiguration* collisionConfiguration = new btDefaultCollisionConfiguration();
	btCollisionDispatcher* dispatcher = new btCollisionDispatcher(collisionConfiguration);

	// The actual physics solver
	btSequentialImpulseConstraintSolver* solver = new btSequentialImpulseConstraintSolver;

	// The world.
	btDiscreteDynamicsWorld* dynamicsWorld = new btDiscreteDynamicsWorld(dispatcher, broadphase, solver, collisionConfiguration);

	bool debug_physics = false;
	btDebugDraw* debug_drawer = new btDebugDraw();
	debug_drawer->setDebugMode(btIDebugDraw::DBG_DrawWireframe | btIDebugDraw::DBG_DrawAabb); // Wireframe + AABB  | btIDebugDraw::DBG_FastWireframe
	dynamicsWorld->setDebugDrawer(debug_drawer);
	SDL_Surface* mapdata = SDL_LoadBMP("testmap.bmp");
	int mwidth = mapdata->w, mlength = mapdata->h;
	roy_assert(mwidth != 0 && mwidth != 0, "Terrain heightmap failed to load image data");
	
	float stroke_scale = 1.0f;
	float height_scale = 10.0f;
	float min_height = -10.0f;
	float max_height = 10.0f;
	// Scale heightmap data
	uint8_t pixel_size = mapdata->format->BytesPerPixel;
	SDL_LockSurface(mapdata);
	float* raw_heightfield_data = (float*)malloc(sizeof(float) * mwidth * mlength);
	for (int i = 0; i < (mwidth * mlength); i++) {
		// get grey value
		unsigned char grey_val;

		SDL_Color* color;
		/* Check the bitdepth of the surface */
		roy_assert(mapdata->format->BitsPerPixel != 8, "Not an 8-bit surface. (uint8_t wont work) \n");
		//mapdata->pixels[(y * mapdata->pitch) + x * mapdata->format->BytesPerPixel];
		uint8_t* pixel = (uint8_t*)mapdata->pixels + i * mapdata->format->BytesPerPixel;
		uint8_t r = pixel[0], g = pixel[1], b = pixel[2];
		raw_heightfield_data[i] = ((float)r / 255.0f) * height_scale;
	}
	SDL_UnlockSurface(mapdata);
	SDL_FreeSurface(mapdata);
	btHeightfieldTerrainShape* heightfield_shape = heightfield_shape =
		new btHeightfieldTerrainShape(
			mwidth, mlength,
			raw_heightfield_data,
			stroke_scale, 
			min_height, max_height,
			1, PHY_FLOAT, false);
	roy_assert(heightfield_shape != NULL, "Terrain failed to initialise btHeightfieldTerrainShape");
	heightfield_shape->setLocalScaling(btVector3(stroke_scale, 1.0, stroke_scale));
	btTransform start_trans;
	start_trans.setIdentity();
	start_trans.setOrigin(btVector3(0, 0, 0));
	// Using motionstate is optional, it provides interpolation capabilities, and only synchronizes 'active' objects
	btDefaultMotionState* motion_state = new btDefaultMotionState(start_trans);
	btRigidBody::btRigidBodyConstructionInfo rb_info(0.0f, motion_state, heightfield_shape, btVector3(0, 0, 0));
	btRigidBody* body = new btRigidBody(rb_info);
	dynamicsWorld->addRigidBody(body);

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
		// bottom square
		// x     y     z
	     0.5f,  0.0f,  0.5f,
	    -0.5f,  0.0f,  0.5f,
	    -0.5f,  0.0f, -0.5f,
	     0.5f,  0.0f, -0.5f,
	     // top square
	     0.5f,  0.5f,  0.5f,
	    -0.5f,  0.5f,  0.5f,
	    -0.5f,  0.5f, -0.5f,
	     0.5f,  0.5f, -0.5f,
	};
	triangle_verts = buffer_new(sizeof(vertices), vertices, BUFFER_VERTEX, USAGE_STATIC, false);	

	unsigned int indices[] = {
		// bottom face
		0, 1, 2,
		2, 3, 0,
		// top face
		4, 5, 6,
		6, 7, 4,
		// back side face
		0, 4, 7,
		7, 3, 0
	};
	triangle_indices = buffer_new(sizeof(indices), indices, BUFFER_INDEX, USAGE_STATIC, false);	

	GLuint VAO;
	glGenVertexArrays(1, &VAO);
	glBindVertexArray(VAO);
	// vertex position attribute
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);
	glBindVertexArray(0);

	mesh_t sphere;
	mesh_sphere(&sphere, 16, 16, 2.0, 1.0, false);
	buffer_t *sphere_verts = buffer_new(sphere.positions.length*sizeof(float), sphere.positions.data, BUFFER_VERTEX, USAGE_STATIC, false);
	buffer_t *sphere_indices = buffer_new(sphere.indices.length*sizeof(unsigned int), sphere.indices.data, BUFFER_INDEX, USAGE_STATIC, false);
	GLuint sphereVAO;
	glGenVertexArrays(1, &sphereVAO);
	glBindVertexArray(sphereVAO);
	// vertex position attribute
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);
	glBindVertexArray(0);

	roy_log(LOG_INFO, "GPU", "buffer memory: %d bytes", gl_gpu_state_buffer_memory());

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

	bool mouse_captured = false;
    int width = 0, height = 0;
    SDL_GetWindowSize(window, &width, &height);
	glViewport(0, 0, width, height);
	glEnable(GL_TEXTURE_2D);
	enum {EASY, HARD};
	static int op = EASY;
	static float value = 0.6f;

	// camera
	camera_t camera;
	camera_new(&camera, PERSPECTIVE, 60., .001, 1000.);
	float pos[4] = {.0, .0, .0, 1.};
	vec3_init(camera.position, pos);

	// model pose
    float model[16];
    mat4_identity(model);
	mat4_translate(model, .0, -0.3, .0);
	
	// main loop 
	int64_t accumulator = 0;
	bool resync = false;
	double update_rate = 60;
	bool unlock_framerate = true;
	int update_multiplicity = 1;
	roy_log(LOG_INFO, "LOOP", "Update rate (variable frame rate) : %f", update_rate);
	const double fixed_dt = 1.f / update_rate;
	int64_t desired_frametime = SDL_GetPerformanceFrequency() / update_rate;
	const int time_history_count = 4;
	int64_t time_averager[4] = { desired_frametime, desired_frametime, desired_frametime, desired_frametime };
	int64_t vsync_maxerror = SDL_GetPerformanceFrequency() * 0.0002;
	int64_t time_60hz = SDL_GetPerformanceFrequency() / 60; //since this is about snapping to common vsync values
	int64_t snap_frequencies[] = {
		time_60hz,          //60fps
		time_60hz * 2,      //30fps
		time_60hz * 3,      //20fps
		time_60hz * 4,      //15fps
		(time_60hz + 1) / 2,  //120fps //120hz, 240hz, or higher need to round up, so that adding 120hz twice guaranteed is at least the same as adding time_60hz once
		(time_60hz+2)/3,  //180fps //that's where the +1 and +2 come from in those equations
		(time_60hz+3)/4,  //240fps //I do not want to snap to anything higher than 120 in my engine, but I left the math in here anyway
	};
	int64_t prev_frame_time = SDL_GetPerformanceCounter();

	SDL_Event event;
	bool quit = false;

	while (!quit) {
		int64_t current_frame_time  = SDL_GetPerformanceCounter();
		int64_t delta_time = current_frame_time  - prev_frame_time;
		prev_frame_time = current_frame_time;
		if (delta_time > desired_frametime * 8) delta_time = desired_frametime; //ignore extra-slow frames
		if (delta_time < 0) delta_time = 0;

		//vsync time snapping
		for (unsigned int snap = 0; snap < 5; snap++) {
			if (llabs(delta_time - snap_frequencies[snap]) < vsync_maxerror) {
				delta_time = snap_frequencies[snap];
				break;
			}
		}

		for (int i = 0; i < time_history_count - 1; i++) {
			time_averager[i] = time_averager[i + 1];
		}

		time_averager[time_history_count - 1] = delta_time;
		delta_time = 0;
		
		for (int i = 0; i < time_history_count; i++) delta_time += time_averager[i];
		
		delta_time /= time_history_count;
		accumulator += delta_time;
		
		//spiral of death protection
		if (accumulator > desired_frametime * 8) resync = true;

		//timer resync if requested
		if (resync) {
			accumulator = 0;
			delta_time = desired_frametime;
			resync = false;
		}

		if (unlock_framerate) {
			int64_t consumed_dt = delta_time;
			while (accumulator >= desired_frametime) {
				SDL_PumpEvents();
				// Previous state = current state
				// Integrate current state
				// FIXED UPDATE
				//game->fixed_update(fixed_dt);

				if (consumed_dt > desired_frametime) { //cap variable update's dt to not be larger than fixed update, and interleave it (so game state can always get animation frames it needs)
					// VARIABLE UPDATE
					//game->variable_update(fixed_dt);

					consumed_dt -= desired_frametime;
				}
				accumulator -= desired_frametime;
			}
			// VARIABLE UPDATE
			//game->variable_update((double)consumed_dt / SDL_GetPerformanceFrequency());

			// GRAPHICS
			//game->render((double)accumulator / desired_frametime, delta_time);
			double dt = ((double)delta_time / (double)SDL_GetPerformanceFrequency());

			// Gather Inputs
			SDL_PumpEvents();
			nk_input_begin(ctx);
	        while (SDL_PollEvent(&event)) {
	            switch (event.type) {
	            case SDL_QUIT:
	                quit = true;
	                break;
	            case SDL_KEYUP:
	            	switch (event.key.keysym.scancode) {
            		case SDL_SCANCODE_ESCAPE: 
		            	mouse_captured = !mouse_captured;
		            	SDL_SetRelativeMouseMode(mouse_captured ? SDL_TRUE : SDL_FALSE);
		            	break;
	            	}
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
			camera_handle_input(&camera, dt);


			// Physics
			dynamicsWorld->stepSimulation(
				dt,						// Time since last step
				1,								// Mas substep count (7)
				btScalar(1.) / btScalar(60.));	// Fixed time step 

			// Graphics Begin
			//game->render(1.0, 1.0);
			glViewport(0, 0, width, height);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
			glClearColor(0.2f, 0.2f, 0.2f, 1.0f);

			// Render
	        gl_use_program(normal_shader.program);
	        
	        float mvp[16], projection[16], view[16];
	        mat4_identity(mvp);

	        //mat4_rotate(model, 100. * dt * (float) M_PI / 180.f,.0,1.,.0);
			
	        mat4_mul(mvp, camera_projection(projection, &camera, width, height));
			mat4_mul(mvp, camera_view(view, &camera));
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
			//glDrawArrays(GL_TRIANGLES, 0, 36);
			gl_gpu_bind_buffer(triangle_indices->type, triangle_indices->id);
			glDrawElements(
				GL_TRIANGLES,
				triangle_indices->size,
				GL_UNSIGNED_INT, // WARNING: This may be different depending on mesh size and will need to be passed to the Mesh at load
				(void*)0); // offset
			glBindVertexArray(0);
			gl_use_program(0);
			glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

			{
				gl_use_program(normal_shader.program);
				float mvp[16], projection[16], view[16];
				mat4_identity(mvp);
				mat4_mul(mvp, camera_projection(projection, &camera, width, height));
				mat4_mul(mvp, camera_view(view, &camera));
				mat4_mul(mvp, model);
				shader_set_uniform(&normal_shader, "MVP", UNIFORM_MATRIX, mvp, 0, 1, sizeof(mvp), "MVP Uniform");
				uint64_t index = map_get(&normal_shader.uniform_map, hash64("MVP", strlen("MVP")));
				roy_assert(index != MAP_NIL, "Cannot find shader uniform!");
				struct uniform_t uniform = normal_shader.uniforms.data[index];
				//shader_set_uniform(normal_shader, "Model", void* data, uint32_t start, uint32_t count, uint32_t size);
				glUniformMatrix4fv(uniform.location, uniform.count, GL_FALSE, (GLfloat*)uniform.value.data);

				glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
				glDisable(GL_CULL_FACE);
				glBindVertexArray(sphereVAO);
				gl_gpu_bind_buffer(sphere_verts->type, sphere_verts->id);
				gl_gpu_bind_buffer(sphere_indices->type, sphere_indices->id);
				glDrawElements(
					GL_TRIANGLES,
					sphere_indices->size,
					GL_UNSIGNED_INT, // WARNING: This may be different depending on mesh size and will need to be passed to the Mesh at load
					(void*)0); // offset
				glBindVertexArray(0);
				gl_use_program(0);
				glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
			}

			
			// Debug physics
			if (debug_physics) {
				mat4_identity(mvp);
				float tm[16], projection[16], view[16];;
				mat4_identity(tm);
				mat4_mul(mvp, camera_projection(projection, &camera, width, height));
				mat4_mul(mvp, camera_view(view, &camera));
				mat4_mul(mvp, tm);
				mat4_init(debug_drawer->mvp, mvp);
				dynamicsWorld->debugDrawWorld();
				debug_drawer->drawLine(btVector3(.0, .0, .0), btVector3(.0, 1., .0), btVector3(1., .0, .0));
			}

			// GUI        
	        if (nk_begin(ctx, "Roy Scene", nk_rect(10, 40, 400, 3*220),
	            NK_WINDOW_BORDER | NK_WINDOW_MOVABLE | NK_WINDOW_SCALABLE | NK_WINDOW_MINIMIZABLE)) { // NK_WINDOW_CLOSABLE | NK_WINDOW_NO_SCROLLBAR | NK_WINDOW_SCALE_LEFT
	            /* fixed widget pixel width */
	            nk_layout_row_static(ctx, 30, 80, 1);
	            if (nk_button_label(ctx, "Physics Visualisation")) {
	                /* event handling */
					debug_physics = !debug_physics;
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

	            nk_layout_row_static(ctx, 30, 200, 1);
	            {
	                char fps_label[64];
		            double fps = 1.0 / dt;
	                snprintf(fps_label, 64, "fps:%.3lf\0", fps);
	                nk_label(ctx, fps_label, NK_TEXT_LEFT);
	            }

				nk_layout_row_static(ctx, 30, 200, 1);
				{
					char cam_angle_label[64];
					snprintf(cam_angle_label, 64, "yaw:%.2f,pitch:%.2f\0", camera.yaw, camera.pitch);
					nk_label(ctx, cam_angle_label, NK_TEXT_LEFT);
				}

				nk_layout_row_static(ctx, 30, 200, 1);
				{
					gui_transform(ctx, "test Matrix", model);
				}

				/* complex color combobox */
				static struct nk_color combo_color = { 130, 50, 50, 255 };
				static struct nk_colorf combo_color2 = { 0.509f, 0.705f, 0.2f, 1.0f };
				if (nk_combo_begin_color(ctx, nk_rgb_cf(combo_color2), nk_vec2(200, 400))) {
					enum color_mode { COL_RGB, COL_HSV };
					static int col_mode = COL_RGB;

					nk_layout_row_dynamic(ctx, 120, 1);
					combo_color2 = nk_color_picker(ctx, combo_color2, NK_RGBA);

					nk_layout_row_dynamic(ctx, 25, 2);
					col_mode = nk_option_label(ctx, "RGB", col_mode == COL_RGB) ? COL_RGB : col_mode;
					col_mode = nk_option_label(ctx, "HSV", col_mode == COL_HSV) ? COL_HSV : col_mode;

					nk_layout_row_dynamic(ctx, 25, 1);
					if (col_mode == COL_RGB) {
						combo_color2.r = nk_propertyf(ctx, "#R:", 0, combo_color2.r, 1.0f, 0.01f, 0.005f);
						combo_color2.g = nk_propertyf(ctx, "#G:", 0, combo_color2.g, 1.0f, 0.01f, 0.005f);
						combo_color2.b = nk_propertyf(ctx, "#B:", 0, combo_color2.b, 1.0f, 0.01f, 0.005f);
						combo_color2.a = nk_propertyf(ctx, "#A:", 0, combo_color2.a, 1.0f, 0.01f, 0.005f);
					}
					else {
						float hsva[4];
						nk_colorf_hsva_fv(hsva, combo_color2);
						hsva[0] = nk_propertyf(ctx, "#H:", 0, hsva[0], 1.0f, 0.01f, 0.05f);
						hsva[1] = nk_propertyf(ctx, "#S:", 0, hsva[1], 1.0f, 0.01f, 0.05f);
						hsva[2] = nk_propertyf(ctx, "#V:", 0, hsva[2], 1.0f, 0.01f, 0.05f);
						hsva[3] = nk_propertyf(ctx, "#A:", 0, hsva[3], 1.0f, 0.01f, 0.05f);
						combo_color2 = nk_hsva_colorfv(hsva);
					}
					nk_combo_end(ctx);
				}

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


			SDL_FlushEvent(SDL_KEYDOWN);
		}
		else {
			roy_assert(false, "I have not implimented unlock framerate because this loop is just for testing atm.");
			while (accumulator >= desired_frametime * update_multiplicity) {
				for (int i = 0; i < update_multiplicity; i++) {
					// FIXED UPDATE
					//game->fixed_update(fixed_dt);

					// VARIABLE UPDATE
					//game->variable_update(fixed_dt);
					
					accumulator -= desired_frametime;
				}
			}

		}
	}

    nk_sdl_shutdown();

	SDL_GL_DeleteContext(gl_context);
	SDL_DestroyWindow(window);
	SDL_Quit();
    return 0;
}