#ifndef GL_H
#define GL_H

#include "shader.h"
#include "mesh.h"

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

// GPU buffer
struct buffer_t {

};

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