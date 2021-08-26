#ifndef MESH_H
#define MESH_H

#include <stdbool.h>

#include "opengl.h"
#include "shader.h"


/* Mesh is a single vertex
and index buffer */
struct mesh_t {
	GLenum draw_mode;
	buffer_t* vertex_buffer;
	buffer_t* index_buffer;
};

/* Model */
struct model_t {

	bool static_body;
};

/* Skeleton */
struct skeleton_t {

};


#endif