#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

// SDL2
#include <SDL2/SDL.h>
//#include <SDL/SDL_opengl.h>

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



#include "test.h"



SDL_Window* window;
SDL_GLContext* gl_context;
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
		fprintf(stderr, "[x] Could not create SDL context: %s", SDL_GetError());
	}

	SDL_GL_SetSwapInterval(1);
	init_glad();

	glEnable(GL_MULTISAMPLE);
	// TODO: Add resizable viewport
	glViewport(0, 0, context_width, context_height);
	
	return context;
}

/*
static void gltf_parse(const void* ptr, uint64_t num_bytes);
static void gltf_parse_buffers(const cgltf_data* gltf);
static void gltf_parse_images(const cgltf_data* gltf);
static void gltf_parse_materials(const cgltf_data* gltf);
static void gltf_parse_meshes(const cgltf_data* gltf);
static void gltf_parse_nodes(const cgltf_data* gltf);
*/
struct nk_canvas {
    struct nk_command_buffer *painter;
    struct nk_vec2 item_spacing;
    struct nk_vec2 panel_padding;
    struct nk_style_item window_background;
};

static void
canvas_begin(struct nk_context *ctx, struct nk_canvas *canvas, nk_flags flags,
    int x, int y, int width, int height, struct nk_color background_color)
{
    /* save style properties which will be overwritten */
    canvas->panel_padding = ctx->style.window.padding;
    canvas->item_spacing = ctx->style.window.spacing;
    canvas->window_background = ctx->style.window.fixed_background;

    /* use the complete window space and set background */
    ctx->style.window.spacing = nk_vec2(0,0);
    ctx->style.window.padding = nk_vec2(0,0);
    ctx->style.window.fixed_background = nk_style_item_color(background_color);

    /* create/update window and set position + size */
    flags = flags & ~NK_WINDOW_DYNAMIC;
    nk_window_set_bounds(ctx, "Window", nk_rect(x, y, width, height));
    nk_begin(ctx, "Window", nk_rect(x, y, width, height), NK_WINDOW_NO_SCROLLBAR|flags);

    /* allocate the complete window space for drawing */
    {struct nk_rect total_space;
    total_space = nk_window_get_content_region(ctx);
    nk_layout_row_dynamic(ctx, total_space.h, 1);
    nk_widget(&total_space, ctx);
    canvas->painter = nk_window_get_canvas(ctx);}
}

static void
canvas_end(struct nk_context *ctx, struct nk_canvas *canvas)
{
    nk_end(ctx);
    ctx->style.window.spacing = canvas->panel_padding;
    ctx->style.window.padding = canvas->item_spacing;
    ctx->style.window.fixed_background = canvas->window_background;
}

void resize_window(SDL_Window* window, int window_width, int window_height) {
    SDL_SetWindowSize(window, window_width, window_height);
    glViewport(0, 0, window_width, window_height);
}

int main(int argc, char* argv[]) {
	window = new_SDL_Window(1200, 800, "SDL-OpenGL-Project");
	gl_context = new_SDL_GLContext(
		window, 
		500, 500, 
		4, 6, // OpenGL major, minor version
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

		// PHYSICS
		dynamicsWorld->stepSimulation(
			0.01,						// Time since last step
			7,								// Mas substep count
			btScalar(1.) / btScalar(60.));	// Fixed time step 

		// GUI        
		struct nk_canvas canvas;
        canvas_begin(ctx, &canvas, 0, 0, 0, width, 30, nk_rgb(250,250,250));
        {
            nk_draw_text(canvas.painter, nk_rect(0, 0, 150, 30), "Roy v0.1", 8, ctx->style.font, nk_rgb(188,174,118), nk_rgb(0,0,0));
        }
        canvas_end(ctx, &canvas);
        

        if (nk_begin(ctx, "Show", nk_rect(50, 50, 220, 220),
            NK_WINDOW_BORDER | NK_WINDOW_MOVABLE | NK_WINDOW_CLOSABLE)) {
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
        glViewport(0, 0, width, height);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glClearColor(0.2f, 0.2f, 0.2f, 1.0f);
        /* IMPORTANT: `nk_sdl_render` modifies some global OpenGL state
         * with blending, scissor, face culling, depth test and viewport and
         * defaults everything back into a default state.
         * Make sure to either a.) save and restore or b.) reset your own state after
         * rendering the UI. */
        nk_sdl_render(NK_ANTI_ALIASING_ON, MAX_VERTEX_MEMORY, MAX_ELEMENT_MEMORY);		SDL_GL_SwapWindow(window);
	}

    nk_sdl_shutdown();

	SDL_GL_DeleteContext(gl_context);
	SDL_DestroyWindow(window);
	SDL_Quit();
    return 0;
}