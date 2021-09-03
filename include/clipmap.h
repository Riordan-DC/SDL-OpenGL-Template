#ifndef _CLIPMAP_H_
#define _CLIPMAP_H_

#include <vector>
#include <string>
#include <sstream>

#include "opengl.h"
#include "linalg.h"

struct cull_block {
	int count;
	int start_index;
	float bound[4][2];
};

typedef arr_t(float) arr_vert_t;
typedef arr_t(GLuint) arr_idx_t;

struct clipmap {
	arr_t(cull_block) blocks;

	GLuint m_vbo[6];
	GLuint m_vao[2];

	int m_min_draw_count;
	int	m_primcount;
	GLsizei	m_draw_count[128];
	GLuint m_draw_starts[128];

	int	m_N;		// clipmap dim (verts)
	int	m_M;		// block size
	int	m_nLevels;
	int	m_heightmap_dim;

	int	m_nInnerIndices;

	float m_quad_size;
	float m_texel_size;
	float m_tex_to_metre;
	float m_metre_to_tex;
};

void clipmap_new(struct clipmap* cmap, int nVerts, float quad_size, int nLevels, int heightmap_dim);
void create_block(struct clipmap* cmap, int vertstart, int width, int height, arr_vert_t *vertices, arr_idx_t *indices);
void cull(struct clipmap* cmap, mat4 mvp, vec2 shift);
void render_inner(struct clipmap* cmap);
void render_levels(struct clipmap* cmap);
void clipmap_delete(struct clipmap* cmap);

void clipmap_new(struct clipmap* cmap, int nVerts, float quad_size, int nLevels, int heightmap_dim) {
	if (((nVerts + 1) & nVerts) != 0) {
		int pot = 1 << int(ceil(log2((float)nVerts + 1.0f)));
		roy_log(LOG_WARN, "CLIPMAP", "nVerts must be an integer  2^k - 1, Rounding up %d -> %d", nVerts, pot - 1);
		nVerts = pot - 1;
	}

	arr_init(&cmap->blocks, realloc);

	cmap->m_N = nVerts;
	cmap->m_nLevels = nLevels;
	cmap->m_quad_size = quad_size;
	cmap->m_heightmap_dim = heightmap_dim;
	cmap->m_min_draw_count = 0;

	//////////////////////////////////////////////////
	// Just some scaling settings
	// 1 unit = 1 metre
	// NB The finest clip level quads should match texel resolution
	// => texelsize = 1.0/texture_dimension
	// The size of the finest quads will determine the physical area a heightmap will represent

	cmap->m_texel_size = 1.0f / (heightmap_dim);
	cmap->m_M = (cmap->m_N + 1) / 4; // block size

	cmap->m_metre_to_tex = cmap->m_texel_size / quad_size;
	cmap->m_tex_to_metre = 1.0f / cmap->m_metre_to_tex;
	
	// Log settings
	roy_log(LOG_INFO, "CLIPMAP", "Clipmap levels:%d", cmap->m_nLevels);
	roy_log(LOG_INFO, "CLIPMAP", "Finest quad size:%f", cmap->m_quad_size);
	roy_log(LOG_INFO, "CLIPMAP", "Vertices per ring-side:%d", cmap->m_N);
	roy_log(LOG_INFO, "CLIPMAP", "Sampling distance:%f", cmap->m_texel_size);
	roy_log(LOG_INFO, "CLIPMAP", "Effective heightmap size:%f m", (float)heightmap_dim * quad_size);

	//Init

	int i, j;
	float texel_size, left, ffar;
	int vmarker, vcount;
	arr_vert_t vertices, vertices_inner;
	arr_t(float) texcoords, texcoords_inner;
	arr_idx_t indices;
	arr_t(GLushort)	indices_inner;
	arr_idx_t cullable;

	arr_init(&vertices, realloc);
	arr_init(&vertices_inner, realloc);
	arr_init(&texcoords, realloc);
	arr_init(&texcoords_inner, realloc);
	arr_init(&indices, realloc);
	arr_init(&indices_inner, realloc);
	arr_init(&cullable, realloc);

	vcount = 0;  // The index of the next vertex (ie the number of vertices so far)

	// Create the inner grid
	left = -(cmap->m_N - 1) * 0.5f;
	ffar = -(cmap->m_N - 1) * 0.5f;
	for (i = 0; i < cmap->m_N; i++) {
		for (j = 0; j < cmap->m_N; j++) {
			float vx = left * quad_size + j * quad_size;
			float vy = ffar * quad_size + i * quad_size;
			float position[2] = { vx, vy };

			float u = 0.5f + left * texel_size + j * texel_size;
			float v = 0.5f + ffar * texel_size + i * texel_size;;
			float tex_coord[2] = { u, v };

			arr_append(&vertices_inner, position, 2);
			arr_append(&texcoords_inner, tex_coord, 2);

			if (i > 0 && j > 0) {
				GLuint idxs[6] = {
					vcount - 1,
					vcount - cmap->m_N,
					vcount - cmap->m_N - 1,

					vcount - cmap->m_N,
					vcount - 1,
					vcount
				};

				arr_append(&indices_inner, idxs, 6);
			}
			vcount++;
		}
	}
	vcount = 0;

	// Create the LOD regions
	//////////////////////////////////
	for (i = 0; i < cmap->m_nLevels; i++)
	{
		quad_size *= 2;
		texel_size *= 2;

		// Create degnerate triangles around the centre top
		for (int j = 0; j < (cmap->m_N - 1) / 2 + 1; j++)
		{
			float vx = left * quad_size / 2 + j * quad_size;
			float vy = ffar * quad_size / 2;
			float u = 0.5f + left * texel_size / 2 + j * texel_size;
			float v = 0.5f + ffar * texel_size / 2;
			float position[2] = { vx, vy };
			float tex_coord[2] = { u, v	};

			if (j > 0)
			{
				float position_2[2] = { vx - 0.5f * quad_size, vy };
				float tex_coord_2[2] = { u - 0.5f * texel_size, v };

				arr_append(&vertices, position_2, 2);
				arr_append(&texcoords, tex_coord_2, 2);

				GLuint idxs[3] = { vcount, vcount + 1, vcount - 1 };
				arr_append(&indices, idxs, 3);
				vcount++;
			}

			arr_append(&vertices, position, 2);
			arr_append(&texcoords, tex_coord, 2);
			vcount++;
		}
		// Create degnerate triangles around the centre bottom
		for (int j = 0; j < (cmap->m_N - 1) / 2 + 1; j++) {
			float vx = left * quad_size / 2 + j * quad_size;
			float vy = ffar * quad_size / 2 + (cmap->m_N - 1) * quad_size / 2;
			float u = 0.5f + left * texel_size / 2 + j * texel_size;
			float v = 0.5f + ffar * texel_size / 2 + (cmap->m_N - 1) * texel_size / 2;

			float position[2] = { vx, vy };
			float tex_coord[2] = { u, v };

			if (j > 0) {
				float position_2[2] = { vx - 0.5f * quad_size, vy };
				float tex_coord_2[2] = { u - 0.5f * texel_size, v };

				arr_append(&vertices, position_2, 2);
				arr_append(&texcoords, tex_coord_2, 2);

				GLuint idxs[3] = { vcount, vcount - 1, vcount + 1 };
				arr_append(&indices, idxs, 3);
				vcount++;
			}

			arr_append(&vertices, position, 2);
			arr_append(&texcoords, tex_coord, 2);
			vcount++;
		}
		// Create degnerate triangles around the centre left
		for (int j = 0; j < (cmap->m_N - 1) / 2 + 1; j++) {
			//vec2 v;
			//vec2 tc;

			float vx = left * quad_size / 2;
			float vy = ffar * quad_size / 2 + j * quad_size;
			float u = 0.5f + left * texel_size / 2;
			float v = 0.5f + ffar * texel_size / 2 + j * texel_size;

			float position[2] = { vx, vy };
			float tex_coord[2] = { u, v };

			if (j > 0) {
				float position_2[2] = { vx, vy - 0.5f * quad_size };
				float tex_coord_2[2] = { u, v - 0.5f * texel_size };

				arr_append(&vertices, position_2, 2);
				arr_append(&texcoords, tex_coord_2, 2);

				GLuint idxs[3] = { vcount, vcount - 1, vcount + 1 };
				arr_append(&indices, idxs, 3);
				vcount++;
			}

			arr_append(&vertices, position, 2);
			arr_append(&texcoords, tex_coord, 2);
			vcount++;
		}
		// Create degnerate triangles around the centre right
		for (int j = 0; j < (cmap->m_N - 1) / 2 + 1; j++) {
			float vx = left * quad_size / 2 + (cmap->m_N - 1) * quad_size / 2;
			float vy = ffar * quad_size / 2 + j * quad_size;
			float u = 0.5f + left * texel_size / 2 + (cmap->m_N - 1) * texel_size / 2;
			float v = 0.5f + ffar * texel_size / 2 + j * texel_size;

			float position[2] = { vx, vy };
			float tex_coord[2] = { u, v };

			if (j > 0) {
				float position_2[2] = { vx, vy - 0.5f * quad_size };
				float tex_coord_2[2] = { u, v - 0.5f * texel_size };

				arr_append(&vertices, position_2, 2);
				arr_append(&texcoords, tex_coord_2, 2);

				GLuint idxs[3] = { vcount, vcount + 1, vcount - 1 };
				arr_append(&indices, idxs, 3);
				vcount++;
			}

			arr_append(&vertices, position, 2);
			arr_append(&texcoords, tex_coord, 2);
			vcount++;
		}
		// Construct the L shape top/bottom strip
		for (int j = 0; j < (cmap->m_N - 1) / 2 + 1; j++) {
			float v1x = left * quad_size / 2 + j * quad_size;;
			float v1y = ffar * quad_size / 2 - quad_size + (i % 2 == 1 ? (cmap->m_N + 1) * quad_size / 2 : 0);
			
			float position[2] = { v1x, v1y };
			float position_2[2] = { v1x, v1y + quad_size };

			float u1 = 0.5f + left * texel_size / 2 + j * texel_size;
			float v1 = 0.5f + ffar * texel_size / 2 - texel_size + (i % 2 == 1 ? (cmap->m_N + 1) * texel_size / 2 : 0);
			
			float tex_coord[2] = { u1, v1 };
			float tex_coord_2[2] = { u1, v1 + texel_size};

			if (j > 0) {
				GLuint idxs[6] = {
					vcount - 1,
					vcount,
					vcount - 2,
					vcount - 1,
					vcount + 1,
					vcount
				};
				arr_append(&indices, idxs, 6);
			}

			arr_append(&vertices, position, 2);
			arr_append(&vertices, position_2, 2);

			arr_append(&texcoords, tex_coord, 2);
			arr_append(&texcoords, tex_coord_2, 2);
			vcount += 2;
		}
		// Construct the L shape left/right strip
		for (int j = 0; j < (cmap->m_N - 1) / 2 + 2; j++) {
			float v1x = left * quad_size / 2 - quad_size + (i % 2 == 0 ? (cmap->m_N + 1) * quad_size / 2 : 0);
			float v1y = ffar * quad_size / 2 + j * quad_size - (i % 2 == 0 ? quad_size : 0);

			float position[2] = { v1x, v1y };
			float position_2[2] = { v1x + quad_size , v1y};

			float u1 = 0.5f + left * texel_size / 2 - texel_size + (i % 2 == 0 ? (cmap->m_N + 1) * texel_size / 2 : 0);
			float v1 = 0.5f + ffar * texel_size / 2 + j * texel_size - (i % 2 == 0 ? texel_size : 0);

			float tex_coord[2] = { u1, v1 };
			float tex_coord_2[2] = { u1 + texel_size , v1};

			if (j > 0) {
				GLuint idxs[6] = {
					vcount,
					vcount - 1,
					vcount - 2,
					vcount,
					vcount + 1,
					vcount - 1
				};
				arr_append(&indices, idxs, 6);
			}

			arr_append(&vertices, position, 2);
			arr_append(&vertices, position_2, 2);

			arr_append(&texcoords, tex_coord, 2);
			arr_append(&texcoords, tex_coord_2, 2);
			
			vcount += 2;
		}


		// Now we're gonna work on the left of this level, so update these variables
		left = left * 0.5f - (cmap->m_M - 1 + i % 2);
		ffar = ffar * 0.5f - (cmap->m_M - 1 + (i + 1) % 2);

		// Create left column of blocks
		vmarker = vcount;
		for (int j = 0; j < cmap->m_N; j++) {
			for (int k = 0; k < cmap->m_M; k++) {
				float vx = left * quad_size + k * quad_size;
				float vy = ffar * quad_size + j * quad_size;
				float u = 0.5f + left * texel_size + k * texel_size;
				float v = 0.5f + ffar * texel_size + j * texel_size;

				float position[2] = { vx, vy };
				float tex_coord[2] = { u, v };

				arr_append(&vertices, position, 2);
				arr_append(&texcoords, tex_coord, 2);
				vcount++;
			}
		}
		create_block(cmap, vmarker + cmap->m_M * (cmap->m_M - 1) * 0, cmap->m_M, cmap->m_M, &vertices, &cullable);
		create_block(cmap, vmarker + cmap->m_M * (cmap->m_M - 1) * 1, cmap->m_M, cmap->m_M, &vertices, &cullable);
		create_block(cmap, vmarker + cmap->m_M * (cmap->m_M - 1) * 2, cmap->m_M, 3, &vertices, &cullable); // fixup
		create_block(cmap, vmarker + cmap->m_M * (cmap->m_M - 1) * 2 + 2 * cmap->m_M, cmap->m_M, cmap->m_M, &vertices, &cullable);
		create_block(cmap, vmarker + cmap->m_M * (cmap->m_M - 1) * 3 + 2 * cmap->m_M, cmap->m_M, cmap->m_M, &vertices, &cullable);

		// Create right column of blocks
		vmarker = vcount;
		for (int j = 0; j < cmap->m_N; j++) {
			for (int k = 0; k < cmap->m_M; k++) {
				float vx = left * quad_size + (k + cmap->m_N - cmap->m_M) * quad_size;
				float vy = ffar * quad_size + j * quad_size;
				float u = 0.5f + left * texel_size + (k + cmap->m_N - cmap->m_M) * texel_size;
				float v = 0.5f + ffar * texel_size + j * texel_size;

				float position[2] = { vx, vy };
				float tex_coord[2] = { u, v };

				arr_append(&vertices, position, 2);
				arr_append(&texcoords, tex_coord, 2);
				vcount++;
			}
		}
		create_block(cmap, vmarker + cmap->m_M * (cmap->m_M - 1) * 0, cmap->m_M, cmap->m_M, &vertices, &cullable);
		create_block(cmap, vmarker + cmap->m_M * (cmap->m_M - 1) * 1, cmap->m_M, cmap->m_M, &vertices, &cullable);
		create_block(cmap, vmarker + cmap->m_M * (cmap->m_M - 1) * 2, cmap->m_M, 3, &vertices, &cullable); // fixup
		create_block(cmap, vmarker + cmap->m_M * (cmap->m_M - 1) * 2 + 2 * cmap->m_M, cmap->m_M, cmap->m_M, &vertices, &cullable);
		create_block(cmap, vmarker + cmap->m_M * (cmap->m_M - 1) * 3 + 2 * cmap->m_M, cmap->m_M, cmap->m_M, &vertices, &cullable);

		// Create top row of blocks
		vmarker = vcount;
		for (int j = 0; j < cmap->m_M; j++)	{
			for (int k = 0; k < cmap->m_M; k++)	{
				float vx = left * quad_size + (k + cmap->m_M - 1) * quad_size;
				float vy = ffar * quad_size + j * quad_size;
				float u = 0.5f + left * texel_size + (k + cmap->m_M - 1) * texel_size;
				float v = 0.5f + ffar * texel_size + j * texel_size;

				float position[2] = { vx, vy };
				float tex_coord[2] = { u, v };

				arr_append(&vertices, position, 2);
				arr_append(&texcoords, tex_coord, 2);
				vcount++;
			}
		}
		create_block(cmap, vmarker, cmap->m_M, cmap->m_M, &vertices, &cullable);
		vmarker = vcount;
		for (int j = 0; j < cmap->m_M; j++)	{
			for (int k = 0; k < 3; k++)	{
				float vx = left * quad_size + (k + 2 * cmap->m_M - 2) * quad_size;
				float vy = ffar * quad_size + j * quad_size;
				float u = 0.5f + left * texel_size + (k + 2 * cmap->m_M - 2) * texel_size;
				float v = 0.5f + ffar * texel_size + j * texel_size;

				float position[2] = { vx, vy };
				float tex_coord[2] = { u, v };

				arr_append(&vertices, position, 2);
				arr_append(&texcoords, tex_coord, 2);
				vcount++;
			}
		}
		create_block(cmap, vmarker, 3, cmap->m_M, &vertices, &cullable);
		vmarker = vcount;
		for (int j = 0; j < cmap->m_M; j++)	{
			for (int k = 0; k < cmap->m_M; k++)	{
				float vx = left * quad_size + (k + 2 * cmap->m_M) * quad_size;
				float vy = ffar * quad_size + j * quad_size;
				float u = 0.5f + left * texel_size + (k + 2 * cmap->m_M) * texel_size;
				float v = 0.5f + ffar * texel_size + j * texel_size;

				float position[2] = { vx, vy };
				float tex_coord[2] = { u, v };

				arr_append(&vertices, position, 2);
				arr_append(&texcoords, tex_coord, 2);
				vcount++;
			}
		}
		create_block(cmap, vmarker, cmap->m_M, cmap->m_M, &vertices, &cullable);

		// Create bottom row of blocks
		vmarker = vcount;
		for (int j = 0; j < cmap->m_M; j++)	{
			for (int k = 0; k < cmap->m_M; k++)	{
				float vx = left * quad_size + (k + cmap->m_M - 1) * quad_size;
				float vy = ffar * quad_size + (j + cmap->m_N - cmap->m_M) * quad_size;
				float u = 0.5f + left * texel_size + (k + cmap->m_M - 1) * texel_size;
				float v = 0.5f + ffar * texel_size + (j + cmap->m_N - cmap->m_M) * texel_size;

				float position[2] = { vx, vy };
				float tex_coord[2] = { u, v };

				arr_append(&vertices, position, 2);
				arr_append(&texcoords, tex_coord, 2);
				vcount++;
			}
		}
		create_block(cmap, vmarker, cmap->m_M, cmap->m_M, &vertices, &cullable);
		vmarker = vcount;
		for (int j = 0; j < cmap->m_M; j++)	{
			for (int k = 0; k < 3; k++)	{
				float vx = left * quad_size + (k + 2 * cmap->m_M - 2) * quad_size;
				float vy = ffar * quad_size + (j + cmap->m_N - cmap->m_M) * quad_size;
				float u = 0.5f + left * texel_size + (k + 2 * cmap->m_M - 2) * texel_size;
				float v = 0.5f + ffar * texel_size + (j + cmap->m_N - cmap->m_M) * texel_size;

				float position[2] = { vx, vy };
				float tex_coord[2] = { u, v };

				arr_append(&vertices, position, 2);
				arr_append(&texcoords, tex_coord, 2);
				vcount++;
			}
		}
		create_block(cmap, vmarker, 3, cmap->m_M, &vertices, &cullable);
		vmarker = vcount;
		for (int j = 0; j < cmap->m_M; j++)	{
			for (int k = 0; k < cmap->m_M; k++)	{
				float vx = left * quad_size + (k + 2 * cmap->m_M) * quad_size;
				float vy = ffar * quad_size + (j + cmap->m_N - cmap->m_M) * quad_size;
				float u = 0.5f + left * texel_size + (k + 2 * cmap->m_M) * texel_size;
				float v = 0.5f + ffar * texel_size + (j + cmap->m_N - cmap->m_M) * texel_size;

				float position[2] = { vx, vy };
				float tex_coord[2] = { u, v };

				arr_append(&vertices, position, 2);
				arr_append(&texcoords, tex_coord, 2);
				vcount++;
			}
		}
		create_block(cmap, vmarker, cmap->m_M, cmap->m_M, &vertices, &cullable);
	}

	cmap->m_min_draw_count = indices.length;
	// Add this offset to all blocks' start indices
	for (int i = 0; i < (int)cmap->blocks.length; i++)
		cmap->blocks.data[i].start_index += cmap->m_min_draw_count * sizeof(GLuint);
	// Append the cullable block indices to the base indices
	//indices.insert(indices.end(), cullable.begin(), cullable.end());
	arr_append(&indices, cullable.data, cullable.length);

	// Add the base indices to the draw list
	cmap->m_draw_count[0] = cmap->m_min_draw_count;
	cmap->m_draw_starts[0] = 0;

	roy_log(LOG_INFO, "CLIPMAP", "Furthest point:%f m", -ffar * quad_size);
	roy_log(LOG_INFO, "CLIPMAP", "Vertex count:%d", (int)vertices.length);
	roy_log(LOG_INFO, "CLIPMAP", "Index count:%d", (int)indices.length);
	roy_log(LOG_INFO, "CLIPMAP", "Clipmap Memory:%f MiB", (float)(texcoords.length + vertices.length + indices.length) * 4.0 / 1024.0f / 1024.0f);

	// OpenGL STUFF
	/////////////////////////////////////

	// Create the vertex array
	glGenVertexArrays(2, cmap->m_vao);

	// Generate three VBOs for vertices, texture coordinates and indices
	glGenBuffers(6, cmap->m_vbo);

	// OUTER LEVELS
	glBindVertexArray(cmap->m_vao[0]);
	// Setup the vertex buffer
	glBindBuffer(GL_ARRAY_BUFFER, cmap->m_vbo[0]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(float) * vertices.length, vertices.data, GL_STATIC_DRAW);
	glVertexAttribPointer((GLuint)0, 2, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(0);
	// Setup the texcoord buffer
	glBindBuffer(GL_ARRAY_BUFFER, cmap->m_vbo[1]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(float) * vertices.length, texcoords.data, GL_STATIC_DRAW);
	glVertexAttribPointer((GLuint)1, 2, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(1);
	// Setup the index buffer
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, cmap->m_vbo[2]);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(GLuint) * indices.length, indices.data, GL_STATIC_DRAW);

	// INNER GRID
	glBindVertexArray(cmap->m_vao[1]);
	// Setup the vertex buffer
	glBindBuffer(GL_ARRAY_BUFFER, cmap->m_vbo[3]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(float) * vertices_inner.length, vertices_inner.data, GL_STATIC_DRAW);
	glVertexAttribPointer((GLuint)0, 2, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(0);
	// Setup the texcoord buffer
	glBindBuffer(GL_ARRAY_BUFFER, cmap->m_vbo[4]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(float) * vertices_inner.length, texcoords_inner.data, GL_STATIC_DRAW);
	glVertexAttribPointer((GLuint)1, 2, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(1);
	// Setup the index buffer
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, cmap->m_vbo[5]);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(GLushort) * indices_inner.length, indices_inner.data, GL_STATIC_DRAW);
	cmap->m_nInnerIndices = indices_inner.length;
}

void create_block(struct clipmap* cmap, int vertstart, int width, int height, arr_vert_t *vertices, arr_idx_t *indices) {
	cull_block block;
	int idx;

	idx = indices->length;
	block.start_index = indices->length * sizeof(GLuint);	// offset into indexbuffer
	block.count = indices->length; // number of indices

	// Form the triangles and push them onto the index list
	for (int y = 0; y < height - 1; y++) {
		for (int x = 0; x < width - 1; x++)	{
			if (x == 0)	{
				GLuint idxs[2] = {
					(y + 0) * width + (x + 0) + vertstart,
					(y + 1) * width + (x + 0) + vertstart
				};

				arr_append(indices, idxs, 2);
			}
			GLuint idxs[2] = {
				(y + 0) * width + (x + 1) + vertstart,
				(y + 1) * width + (x + 1) + vertstart
			};
			arr_append(indices, idxs, 2);
		}
		// degens to start new row
		if (y < height - 2)	{
			GLuint idxs[2] = {
				(y + 1) * width + (width - 1) + vertstart,
				(y + 1) * width + (0) + vertstart
			};
			arr_append(indices, idxs, 2);
		}
	}
	block.count = indices->length - block.count;
	
	int row = (width - 1) * 2 + 2;
	block.bound[0][0] = vertices->data[indices->data[idx]];
	block.bound[0][1] = vertices->data[indices->data[idx]+1];

	block.bound[1][0] = vertices->data[indices->data[idx + row - 2]];
	block.bound[1][1] = vertices->data[indices->data[idx + row - 2]+1];

	block.bound[2][0] = vertices->data[indices->data[idx + block.count + 1 - row]];
	block.bound[2][1] = vertices->data[indices->data[idx + block.count + 1 - row]+1];

	block.bound[3][0] = vertices->data[indices->data[idx + block.count - 1]];
	block.bound[3][1] = vertices->data[indices->data[idx + block.count - 1]+1];

	arr_append(&cmap->blocks, &block, 1);
}

/*

void cull(struct clipmap* cmap, mat4 mvp, vec2 shift) {
	// initialise primcount to 1 for base indices
	m_primcount = 1;
	for (int i = 0; i < (int)blocks.size(); i++)
	{
		cull_block& block = blocks[i];

		for (int j = 0; j < 4; j++)
		{
			vec2& v = block.bound[j];
			vector4 frag = mvp * vector4(v.x + shift.x, -20.0f, v.y + shift.y, 1.0f);
			vector4 NDC = frag * (1.0f / frag.w);

			// if screen x is neither > 1.0 nor < -1.0
			// remember v.y is the z coordinate, the frag's x and y would still need to be divided
			// by the w-coordinate (which is w=-z for perspective) so just mult both sides
			if (!(NDC.z < -1.0f || NDC.z > 1.0f || NDC.x < -1.2f || NDC.x > 1.2f))
			{
				m_draw_count[m_primcount] = blocks[i].count;
				m_draw_starts[m_primcount] = blocks[i].start_index;
				m_primcount++;
				break;
			}
		}
	}
}

void render_inner(struct clipmap* cmap) {
	glBindVertexArray(m_vao[1]);
	glDrawElements(GL_TRIANGLES, m_nInnerIndices, GL_UNSIGNED_SHORT, 0);
}

void render_levels(struct clipmap* cmap) {
	glBindVertexArray(m_vao[0]);
	glDrawElements(GL_TRIANGLES, m_draw_count[0], GL_UNSIGNED_INT, (GLvoid*)(m_draw_starts[0]));
	for (int i = 1; i < m_primcount; i++)
		glDrawElements(GL_TRIANGLE_STRIP, m_draw_count[i], GL_UNSIGNED_INT, (GLvoid*)(m_draw_starts[i]));
}

void clipmap_delete(struct clipmap* cmap) {
	glDisableVertexAttribArray(0);
	glDisableVertexAttribArray(1);
	glDisableVertexAttribArray(2);
	glDeleteBuffers(6, m_vbo);
	glDeleteVertexArrays(2, m_vao);
}
*/
#endif
