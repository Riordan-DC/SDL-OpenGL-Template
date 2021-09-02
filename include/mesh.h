#ifndef MESH_H
#define MESH_H

#include <stdbool.h>

#include "opengl.h"
#include "texture.h"
#include "shader.h"

/*
* Mesh is designed for all 3D data structures
* A simple mesh is just a container for all
* 3D data. Model uses meshes to store its data
* and manages the GPU buffers of this data.
*/

// Mesh is a container for CPU-side 3D data
struct mesh_t {

};





/* Model */
struct model_t {
	struct mest_t *meshes;
	GLenum draw_mode;
	buffer_t* vertex_buffer;
	buffer_t* index_buffer;
	bool is_static;
};

/* Skeleton */
struct skeleton_t {

};


#endif