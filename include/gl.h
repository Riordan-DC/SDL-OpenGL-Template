#ifndef GL_H
#define GL_H

#include "opengl.h"
#include "shader.h"
#include "mesh.h"
#include "linalg.h"

/* GL (graphics lib) manages all
graphics operations. data produces
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

// Window
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
	float view_matrix[16];
	float projection_matrix[16];
	enum camera_type type;
};

int camera_new(camera_t *camera, enum camera_type type, float fov, float aspect) {
	// fov: field of view, in degrees
	// aspect: width / height of viewport
	camera->type = type;
	if (type == PERSPECTIVE) {
		mat4_perspective(camera->projection_matrix, .01f, 100.f, fov * (float) M_PI / 180.f, aspect);
	} else {
		mat4_orthographic(camera->projection_matrix, -1.0, 1.0, 1.0, -1.0, .01f, 100.f);
	}
    mat4_identity(camera->view_matrix);
    return 0;
}

void camera_look_at(camera_t *camera, vec3 pos) {
	if (camera->type == PERSPECTIVE) {
		float cam_pos[3];
		float up[3] = {.0, 1., .0};
		mat4_getPosition(camera->projection_matrix, cam_pos);
		mat4_lookAt(camera->projection_matrix, cam_pos, pos, up);
	} else {
		roy_log(LOG_WARN, "GL", "camera Look At is unimplimented for ORTHOGRAPHIC type cameras");
	}
}

// draw_call
struct draw_call_t {

};

// render target
struct viewport_t {
	// sky gradient
	// buffers: bloom
	// camera
	// shadows
	// objects
};

// point light

// spot light

// directional light

#endif GL_H