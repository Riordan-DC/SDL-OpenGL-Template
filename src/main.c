#include <stdio.h>
#include <stdbool.h>

// SDL2
#include <SDL2/SDL.h>
//#include <SDL/SDL_opengl.h>

#include "glad.h"

#include "test.h"

SDL_Window* window;
SDL_GLContext* context;
int SDL_Window_flags = SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE;

void init_glad(void) {
	// Load OpenGL function pointers
	// This version of GLAD does NOT include a loader:
	//      gladLoadGL();
	if (!gladLoadGLLoader((GLADloadproc)SDL_GL_GetProcAddress)) {
		printf("[x] Failed to initialize GLAD");
		abort();
	}

	glEnable(GL_CULL_FACE);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_MULTISAMPLE);
	fprintf(stderr, "[-] OpenGL Version [%s]\n", glGetString(GL_VERSION));
	fprintf(stderr, "[-] CPU Cache line size [%d bytes]\n", SDL_GetCPUCacheLineSize());
}

SDL_Window* new_SDL_Window(int screen_width, int screen_height, char* window_name) {
	int error = SDL_Init(SDL_INIT_VIDEO); //SDL_InitSubSystem(SDL_INIT_VIDEO);
	
	if (error == -1) {
		// SDL Error
		fprintf(stderr, "[x] Cannot initialize SDL video!");
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
		fprintf(stderr, "[x] Could not create SDL window: %s", SDL_GetError());
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

	SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, msaa_samples);
	SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, msaa_bufs);
	SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 28); // Often 32-bit is the max. WARNING: Increasing can decrease performance. Default: 16

	SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_ALPHA_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);

#ifdef __APPLE__
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, SDL_GL_CONTEXT_FORWARD_COMPATIBLE_FLAG);
	SDL_SetHint(SDL_HINT_VIDEO_HIGHDPI_DISABLED, "1");
#endif
	// Turn on double buffering with a 24bit Z buffer.
	// You may need to change this to 16 or 32 for your system
	 //SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
	 //SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);

	SDL_GLContext* context = (SDL_GLContext*)SDL_GL_CreateContext(window);

	if (context == NULL) {
		fprintf(stderr, "[x] Could not create SDL context: %s", SDL_GetError());
	}

	SDL_GL_SetSwapInterval(1);
	init_glad();

	glEnable(GL_MULTISAMPLE);
	// TODO: Add resizable viewport
	glViewport(0, 0, context_width, context_height);
	
	return context;
}


int main(int argc, char* argv[]) {
	window = new_SDL_Window(500, 500, "SDL-OpenGL-Project");
	context = new_SDL_GLContext(window, 500, 500, 4, 1, 4, 1);

	SDL_Event event;
	bool quit = false;
	while (!quit) {
		while (SDL_PollEvent(&event)) {
			if (event.type == SDL_QUIT) {
				quit = true;
			}
		}

		glEnable(GL_DEPTH_TEST);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glClearColor(0,0,0,1);


		SDL_GL_SwapWindow(window);
	}

	SDL_GL_DeleteContext(context);
	SDL_DestroyWindow(window);
	SDL_Quit();
    return 0;
}
