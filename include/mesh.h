#ifndef MESH_H
#define MESH_H

#include <stdbool.h>

#include "util.h"
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
    arr_t(float) positions;
    arr_t(float) uvs;
    arr_t(float) normals;
    arr_t(unsigned int) indices;
};

// procedural meshes
void mesh_sphere(struct mesh_t* mesh, unsigned int radial_segments, unsigned int rings, float height, float radius, bool is_hemisphere) {
    arr_init(&mesh->positions, realloc);
    arr_init(&mesh->uvs, realloc);
    arr_init(&mesh->normals, realloc);
    arr_init(&mesh->indices, realloc);

	int i, j, prevrow, thisrow, point;
	float x, y, z;

	float scale = height * (is_hemisphere ? 1.0 : 0.5);
	point = 0;

	thisrow = 0;
	prevrow = 0;
	for (j = 0; j <= (rings + 1); j++) {
		float v = j;
		float w;

		v /= (rings + 1);
		w = sinf(M_PI * v);
		y = scale * cosf(M_PI * v);

		for (i = 0; i <= radial_segments; i++) {
			float u = i;
			u /= radial_segments;

			x = sinf(u * TAU);
			z = cosf(u * TAU);

			if (is_hemisphere && y < 0.0) {
                float position[3] = { x * radius * w, 0.0, z * radius * w };
                float normals[3] = { 0.0, -1.0, 0.0 };
                arr_append(&mesh->positions, position, 3);
                arr_append(&mesh->normals, normals, 3);
			}
			else {
                float position[3] = { x * radius * w, y, z * radius * w };
                float normals[3] = { x * radius * w * scale, y / scale, z * radius * w * scale };
                arr_append(&mesh->positions, position, 3);
                arr_append(&mesh->normals, normals, 3);
			};

            float uv[2] = { u, v };
            arr_append(&mesh->uvs, uv, 2);

			point++;

			if (i > 0 && j > 0) {
                unsigned int idxs[6] = {
                    prevrow + i - 1,
                    prevrow + i,
                    thisrow + i - 1,
                    prevrow + i,
                    thisrow + i,
                    thisrow + i - 1
                };
                arr_append(&mesh->indices, idxs, 6);
			}
		}
		prevrow = thisrow;
		thisrow = point;
	}
}


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