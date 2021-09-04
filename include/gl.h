/* GL (graphics lib) manages all
graphics operations. Game data produces
draw call which combined with
a viewport renders a frame. A viewport
managers render state. render data
describes input for a single material.
rendering is a multistep process
where objects are

TODO: functions for adding:
- adding a sky
- adding buffers
	- some default buffers are required for a basic pipeline
	but more buffers are added for more custom pipelines.

- adding objects
- adding a camera
- adding shadows

- building a default viewport */


#ifndef GL_H
#define GL_H

#include "opengl.h"
#include "shader.h"
#include "mesh.h"
#include "linalg.h"

// Window creation and managment
SDL_Window* SDL_Window_new(int screen_width, int screen_height, char* window_name, int SDL_Window_flags) {
	int error = SDL_Init(SDL_INIT_VIDEO); //SDL_InitSubSystem(SDL_INIT_VIDEO);
	
	if (error == -1) {
		// SDL Error
		roy_log(LOG_ERROR, "SDL_WINDOW", "Cannot initialize SDL video!");
	}

	SDL_Window* window = SDL_CreateWindow(
		window_name, 				// window title
		SDL_WINDOWPOS_CENTERED, 	// x position
		SDL_WINDOWPOS_CENTERED, 	// y position
		screen_width, 				// width, in pixels
		screen_height,				// height, in pixels
		SDL_Window_flags			// flags
	);

	if (window == NULL) {
		// SDL Error
		roy_log(LOG_ERROR, "SDL_WINDOW", "Could not create SDL window: %s", SDL_GetError());
	}
	
	SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "1");
	SDL_SetHint(SDL_HINT_RENDER_VSYNC, "1");
	return window;
}

SDL_GLContext* new_SDL_GLContext(SDL_Window* window, int context_width, int context_height, int openGL_major, int openGL_minor, int msaa_samples, int msaa_bufs) {
	SDL_GL_SetAttribute(SDL_GL_SHARE_WITH_CURRENT_CONTEXT, 1);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE); //OpenGL core profile
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, openGL_major);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, openGL_minor);

	SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "1");


	SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, msaa_samples);
	SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, msaa_bufs);

	// Often 32-bit is the max. WARNING: Increasing can decrease performance. Default: 16
	// Changing to 24-bits on Linux crashes
	SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 16); 

	SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 5);
	SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 5);
	SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 5);
	SDL_GL_SetAttribute(SDL_GL_ALPHA_SIZE, 0);

	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);


#ifdef __APPLE__
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, SDL_GL_CONTEXT_FORWARD_COMPATIBLE_FLAG);
	SDL_SetHint(SDL_HINT_VIDEO_HIGHDPI_DISABLED, "1");
#elif __POSIX__
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, SDL_GL_CONTEXT_FORWARD_COMPATIBLE_FLAG);
#endif
	// Turn on double buffering with a 24bit Z buffer.
	// You may need to change this to 16 or 32 for your system
	 //SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
	 //SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);

	SDL_GLContext* context = (SDL_GLContext*)SDL_GL_CreateContext(window);

	if (context == NULL) {
		roy_log(LOG_ERROR, "SDL_GL", "Could not create SDL context: %s", SDL_GetError());
	}

	SDL_GL_SetSwapInterval(1);
	init_glad();

	glEnable(GL_MULTISAMPLE);
	// TODO: Add resizable viewport
	glViewport(0, 0, context_width, context_height);
	
	return context;
}

// Camera
enum camera_type {
	PERSPECTIVE,
	ORTHOGRAPHIC
};

struct camera_t {
	float fov;
	float near;
	float far;
	float position[4];

	float yaw;
	float pitch;
	float roll;
	enum camera_type type;
};

void camera_basis(camera_t* camera, vec3 front, vec3 right, vec3 up) {
	front[0] = cos(RAD(camera->yaw)) * cos(RAD(camera->pitch));
	front[1] = sin(RAD(camera->pitch));
	front[2] = sin(RAD(camera->yaw)) * cos(RAD(camera->pitch));
	vec3_normalize(front);

	float world_up[4] = { .0, 1., .0, 1. };
	vec3_init(right, front);
	vec3_cross(right, world_up);
	vec3_normalize(right);

	vec3_init(up, right);
	vec3_cross(up, front);
	vec3_normalize(up);
}

mat4 camera_view(mat4 view_matrix, camera_t *camera) {
	float front[4], right[4], up[4];
	camera_basis(camera, front, right, up);

	view_matrix[12] = camera->position[0];
	view_matrix[13] = camera->position[1];
	view_matrix[14] = camera->position[2];

	float look_dir[4] = { .0, .0, .0, 1. };
	vec3_init(look_dir, camera->position);
	vec3_add(look_dir, front);

	mat4_lookAt(view_matrix, camera->position, look_dir, up);

	return view_matrix;
}

mat4 camera_projection(mat4 projection_matrix, camera_t* camera, int width, int height) {
	if (camera->type == PERSPECTIVE) {
		mat4_perspective(projection_matrix, camera->near, camera->far, RAD(camera->fov), (float)width / (float)height);
	}
	else {
		roy_assert(0, "Not implimented orthographic camera");
		mat4_orthographic(projection_matrix, -1.0, 1.0, 1.0, -1.0, .01f, camera->far);
	}
	return projection_matrix;
}


void camera_new(camera_t *camera, enum camera_type type, float fov, float near, float far) {
	// fov: field of view, in degrees
	camera->type = type;
	camera->fov = fov;
	camera->near = near;
	camera->far = far;

	camera->position[0] = .0;
	camera->position[1] = .0;
	camera->position[2] = .0;
	camera->position[3] = 1.; // w

    camera->yaw = -90.;
    camera->pitch = .0;
	camera->roll = .0;
}

mat4 camera_look_at(mat4 view_matrix, camera_t *camera, vec3 pos) {
	float front[4], right[4], up[4];
	camera_basis(camera, front, right, up);
	if (camera->type == PERSPECTIVE) {
		mat4_lookAt(view_matrix, camera->position, pos, up);
	} else {
		roy_log(LOG_WARN, "GL", "camera Look At is unimplimented for ORTHOGRAPHIC type cameras");
	}
	return view_matrix;
}

/*
void camera_screen_space_position(int* screen_x, int* screen_y, const vec3 world_position,
	const camera_t *camera, int width, int height) {
	VMatrix view, proj, viewproj;
	ComputeViewMatrix(&view, camera);
	ComputeProjectionMatrix(&proj, camera, width, height);
	MatrixMultiply(proj, view, viewproj);

	Vector vecScreenPos;
	Vector3DMultiplyPositionProjective(viewproj, vecWorldPosition, vecScreenPos);

	pScreenPosition->x = (vecScreenPos.x + 1.0f) * width / 2.0f;
	pScreenPosition->y = (-vecScreenPos.y + 1.0f) * height / 2.0f;
}*/

// draw call format
struct draw_call_t {

};

// render target
struct render_target_t {
	// sky gradient
	// buffers: bloom
	// camera
	// shadows
	// objects
};

// point light

// spot light

// directional light

/*
// Build the framebuffer.
GLuint framebuffer;
glGenFramebuffers(1, &framebuffer);
glBindFramebuffer(GL_FRAMEBUFFER, (GLuint)framebuffer);
glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture_map, 0);
glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depth_texture, 0);

GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
if (status != GL_FRAMEBUFFER_COMPLETE)
	// Error

glBindFramebuffer(GL_FRAMEBUFFER, 0);
*/

#endif GL_H