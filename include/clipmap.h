/*****************************************************************************
 * Header: clipmap
 *
 * Copyright � 2010
 * Authors: Andrew Flower & Justin Crause
 * Emails:	andrew.flower@gmail.com & juzzwuzz@gmail.com
 *****************************************************************************/

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

struct clipmap {
	vector<cull_block> blocks;

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

void clipmap_new(struct clipmap* cmap, int nVerts, float quad_size, int levels, int heightmap_dim) {
	if (((nVerts + 1) & nVerts) != 0) {
		int pot = 1 << int(ceil(log2((float)nVerts + 1.0f)));
		roy_log(LOG_WARN, "Warning\n\tnVerts must be an integer  2^k - 1, Rounding up %d -> %d\n\t\t\t\t", nVerts, pot - 1);
		nVerts = pot - 1;
	}

	m_N = nVerts;
	m_nLevels = nLevels;
	m_quad_size = quad_size;
	m_heightmap_dim = heightmap_dim;
	m_min_draw_count = 0;

	//////////////////////////////////////////////////
	// Just some scaling settings
	// 1 unit = 1 metre
	// NB The finest clip level quads should match texel resolution
	// => texelsize = 1.0/texture_dimension
	// The size of the finest quads will determine the physical area a heightmap will represent

	m_texel_size = 1.0f / (heightmap_dim);
	m_M = (m_N + 1) / 4; // block size

	m_metre_to_tex = m_texel_size / quad_size;
	m_tex_to_metre = 1.0f / m_metre_to_tex;
	
	// Log settings
	roy_log(LOG_INFO, "Clipmap levels:%d", m_nLevels);
	roy_log(LOG_INFO, "Finest quad size:%d", m_quad_size);
	roy_log(LOG_INFO, "Vertices per ring-side:%d", m_N);
	roy_log(LOG_INFO, "Sampling distance:%d", m_texel_size);
	roy_log(LOG_INFO, "Effective heightmap size:%d m", heightmap_dim * quad_size);

	//Init

	int i, j;
	float quad_size, texel_size, left, ffar;
	int vmarker, vcount;
	std::vector <vec2>	vertices, vertices_inner;
	std::vector <vec2>	texcoords, texcoords_inner;
	std::vector <GLuint>	indices;
	std::vector <GLushort>	indices_inner;
	std::vector <GLuint>	cullable;

	quad_size = m_quad_size;
	texel_size = m_texel_size;

	vcount = 0;  // The index of the next vertex (ie the number of vertices so far)

	// Create the inner grid
	left = -(m_N - 1) * 0.5f;
	ffar = -(m_N - 1) * 0.5f;
	for (i = 0; i < m_N; i++)
	{
		for (j = 0; j < m_N; j++)
		{
			vec2 v;
			vec2 tc;

			v.x = left * quad_size + j * quad_size;
			v.y = ffar * quad_size + i * quad_size;
			tc.x = 0.5f + left * texel_size + j * texel_size;
			tc.y = 0.5f + ffar * texel_size + i * texel_size;;

			vertices_inner.push_back(v);
			texcoords_inner.push_back(tc);
			if (i > 0 && j > 0)
			{
				indices_inner.push_back(vcount - 1);
				indices_inner.push_back(vcount - m_N);
				indices_inner.push_back(vcount - m_N - 1);

				indices_inner.push_back(vcount - m_N);
				indices_inner.push_back(vcount - 1);
				indices_inner.push_back(vcount);
			}
			vcount++;
		}
	}
	vcount = 0;

	// Create the LOD regions
	//////////////////////////////////
	for (i = 0; i < m_nLevels; i++)
	{
		quad_size *= 2;
		texel_size *= 2;

		// Create degnerate triangles around the centre top
		for (int j = 0; j < (m_N - 1) / 2 + 1; j++)
		{
			vec2 v;
			vec2 tc;

			v.x = left * quad_size / 2 + j * quad_size;
			v.y = ffar * quad_size / 2;
			tc.x = 0.5f + left * texel_size / 2 + j * texel_size;
			tc.y = 0.5f + ffar * texel_size / 2;

			if (j > 0)
			{
				vec2 vm = v;
				vm.x -= 0.5f * quad_size;
				vec2 tcm = tc;
				tcm.x -= 0.5f * texel_size;
				vertices.push_back(vm);
				texcoords.push_back(tcm);

				indices.push_back(vcount);
				indices.push_back(vcount + 1);
				indices.push_back(vcount - 1);
				vcount++;
			}

			vertices.push_back(v);
			texcoords.push_back(tc);
			vcount++;
		}
		// Create degnerate triangles around the centre bottom
		for (int j = 0; j < (m_N - 1) / 2 + 1; j++)
		{
			vec2 v;
			vec2 tc;

			v.x = left * quad_size / 2 + j * quad_size;
			v.y = ffar * quad_size / 2 + (m_N - 1) * quad_size / 2;
			tc.x = 0.5f + left * texel_size / 2 + j * texel_size;
			tc.y = 0.5f + ffar * texel_size / 2 + (m_N - 1) * texel_size / 2;

			if (j > 0)
			{
				vec2 vm = v;
				vm.x -= 0.5f * quad_size;
				vec2 tcm = tc;
				tcm.x -= 0.5f * texel_size;
				vertices.push_back(vm);
				texcoords.push_back(tcm);

				indices.push_back(vcount);
				indices.push_back(vcount - 1);
				indices.push_back(vcount + 1);
				vcount++;
			}

			vertices.push_back(v);
			texcoords.push_back(tc);
			vcount++;
		}
		// Create degnerate triangles around the centre left
		for (int j = 0; j < (m_N - 1) / 2 + 1; j++)
		{
			vec2 v;
			vec2 tc;

			v.x = left * quad_size / 2;
			v.y = ffar * quad_size / 2 + j * quad_size;
			tc.x = 0.5f + left * texel_size / 2;
			tc.y = 0.5f + ffar * texel_size / 2 + j * texel_size;

			if (j > 0)
			{
				vec2 vm = v;
				vm.y -= 0.5f * quad_size;
				vec2 tcm = tc;
				tcm.y -= 0.5f * texel_size;
				vertices.push_back(vm);
				texcoords.push_back(tcm);

				indices.push_back(vcount);
				indices.push_back(vcount - 1);
				indices.push_back(vcount + 1);
				vcount++;
			}

			vertices.push_back(v);
			texcoords.push_back(tc);
			vcount++;
		}
		// Create degnerate triangles around the centre right
		for (int j = 0; j < (m_N - 1) / 2 + 1; j++)
		{
			vec2 v;
			vec2 tc;

			v.x = left * quad_size / 2 + (m_N - 1) * quad_size / 2;
			v.y = ffar * quad_size / 2 + j * quad_size;
			tc.x = 0.5f + left * texel_size / 2 + (m_N - 1) * texel_size / 2;
			tc.y = 0.5f + ffar * texel_size / 2 + j * texel_size;

			if (j > 0)
			{
				vec2 vm = v;
				vm.y -= 0.5f * quad_size;
				vec2 tcm = tc;
				tcm.y -= 0.5f * texel_size;
				vertices.push_back(vm);
				texcoords.push_back(tcm);

				indices.push_back(vcount);
				indices.push_back(vcount + 1);
				indices.push_back(vcount - 1);
				vcount++;
			}

			vertices.push_back(v);
			texcoords.push_back(tc);
			vcount++;
		}
		// Construct the L shape top/bottom strip
		for (int j = 0; j < (m_N - 1) / 2 + 1; j++)
		{
			vec2 v1, v2;
			vec2 tc1, tc2;

			v1.x = left * quad_size / 2 + j * quad_size;;
			v1.y = ffar * quad_size / 2 - quad_size + (i % 2 == 1 ? (m_N + 1) * quad_size / 2 : 0);
			v2 = v1;
			v2.y += quad_size;
			tc1.x = 0.5f + left * texel_size / 2 + j * texel_size;
			tc1.y = 0.5f + ffar * texel_size / 2 - texel_size + (i % 2 == 1 ? (m_N + 1) * texel_size / 2 : 0);
			tc2 = tc1;
			tc2.y += texel_size;

			if (j > 0)
			{
				indices.push_back(vcount - 1);
				indices.push_back(vcount);
				indices.push_back(vcount - 2);
				indices.push_back(vcount - 1);
				indices.push_back(vcount + 1);
				indices.push_back(vcount);
			}
			vertices.push_back(v1);
			vertices.push_back(v2);
			texcoords.push_back(tc1);
			texcoords.push_back(tc2);
			vcount += 2;
		}
		// Construct the L shape left/right strip
		for (int j = 0; j < (m_N - 1) / 2 + 2; j++)
		{
			vec2 v1, v2;
			vec2 tc1, tc2;

			v1.x = left * quad_size / 2 - quad_size + (i % 2 == 0 ? (m_N + 1) * quad_size / 2 : 0);
			v1.y = ffar * quad_size / 2 + j * quad_size - (i % 2 == 0 ? quad_size : 0);
			v2 = v1;
			v2.x += quad_size;
			tc1.x = 0.5f + left * texel_size / 2 - texel_size + (i % 2 == 0 ? (m_N + 1) * texel_size / 2 : 0);
			tc1.y = 0.5f + ffar * texel_size / 2 + j * texel_size - (i % 2 == 0 ? texel_size : 0);
			tc2 = tc1;
			tc2.x += texel_size;

			if (j > 0)
			{
				indices.push_back(vcount);
				indices.push_back(vcount - 1);
				indices.push_back(vcount - 2);
				indices.push_back(vcount);
				indices.push_back(vcount + 1);
				indices.push_back(vcount - 1);
			}
			vertices.push_back(v1);
			vertices.push_back(v2);
			texcoords.push_back(tc1);
			texcoords.push_back(tc2);
			vcount += 2;
		}


		// Now we're gonna work on the left of this level, so update these variables
		left = left * 0.5f - (m_M - 1 + i % 2);
		ffar = ffar * 0.5f - (m_M - 1 + (i + 1) % 2);

		// Create left column of blocks
		vmarker = vcount;
		for (int j = 0; j < m_N; j++)
		{
			for (int k = 0; k < m_M; k++)
			{
				vec2 v;
				vec2 tc;

				v.x = left * quad_size + k * quad_size;
				v.y = ffar * quad_size + j * quad_size;
				tc.x = 0.5f + left * texel_size + k * texel_size;
				tc.y = 0.5f + ffar * texel_size + j * texel_size;

				vertices.push_back(v);
				texcoords.push_back(tc);
				vcount++;
			}
		}
		create_block(vmarker + m_M * (m_M - 1) * 0, m_M, m_M, vertices, cullable);
		create_block(vmarker + m_M * (m_M - 1) * 1, m_M, m_M, vertices, cullable);
		create_block(vmarker + m_M * (m_M - 1) * 2, m_M, 3, vertices, cullable); // fixup
		create_block(vmarker + m_M * (m_M - 1) * 2 + 2 * m_M, m_M, m_M, vertices, cullable);
		create_block(vmarker + m_M * (m_M - 1) * 3 + 2 * m_M, m_M, m_M, vertices, cullable);

		// Create right column of blocks
		vmarker = vcount;
		for (int j = 0; j < m_N; j++)
		{
			for (int k = 0; k < m_M; k++)
			{
				vec2 v;
				vec2 tc;

				v.x = left * quad_size + (k + m_N - m_M) * quad_size;
				v.y = ffar * quad_size + j * quad_size;
				tc.x = 0.5f + left * texel_size + (k + m_N - m_M) * texel_size;
				tc.y = 0.5f + ffar * texel_size + j * texel_size;

				vertices.push_back(v);
				texcoords.push_back(tc);
				vcount++;
			}
		}
		create_block(vmarker + m_M * (m_M - 1) * 0, m_M, m_M, vertices, cullable);
		create_block(vmarker + m_M * (m_M - 1) * 1, m_M, m_M, vertices, cullable);
		create_block(vmarker + m_M * (m_M - 1) * 2, m_M, 3, vertices, cullable); // fixup
		create_block(vmarker + m_M * (m_M - 1) * 2 + 2 * m_M, m_M, m_M, vertices, cullable);
		create_block(vmarker + m_M * (m_M - 1) * 3 + 2 * m_M, m_M, m_M, vertices, cullable);

		// Create top row of blocks
		vmarker = vcount;
		for (int j = 0; j < m_M; j++)
		{
			for (int k = 0; k < m_M; k++)
			{
				vec2 v;
				vec2 tc;

				v.x = left * quad_size + (k + m_M - 1) * quad_size;
				v.y = ffar * quad_size + j * quad_size;
				tc.x = 0.5f + left * texel_size + (k + m_M - 1) * texel_size;
				tc.y = 0.5f + ffar * texel_size + j * texel_size;

				vertices.push_back(v);
				texcoords.push_back(tc);
				vcount++;
			}
		}
		create_block(vmarker, m_M, m_M, vertices, cullable);
		vmarker = vcount;
		for (int j = 0; j < m_M; j++)
		{
			for (int k = 0; k < 3; k++)
			{
				vec2 v;
				vec2 tc;

				v.x = left * quad_size + (k + 2 * m_M - 2) * quad_size;
				v.y = ffar * quad_size + j * quad_size;
				tc.x = 0.5f + left * texel_size + (k + 2 * m_M - 2) * texel_size;
				tc.y = 0.5f + ffar * texel_size + j * texel_size;

				vertices.push_back(v);
				texcoords.push_back(tc);
				vcount++;
			}
		}
		create_block(vmarker, 3, m_M, vertices, cullable);
		vmarker = vcount;
		for (int j = 0; j < m_M; j++)
		{
			for (int k = 0; k < m_M; k++)
			{
				vec2 v;
				vec2 tc;

				v.x = left * quad_size + (k + 2 * m_M) * quad_size;
				v.y = ffar * quad_size + j * quad_size;
				tc.x = 0.5f + left * texel_size + (k + 2 * m_M) * texel_size;
				tc.y = 0.5f + ffar * texel_size + j * texel_size;

				vertices.push_back(v);
				texcoords.push_back(tc);
				vcount++;
			}
		}
		create_block(vmarker, m_M, m_M, vertices, cullable);

		// Create bottom row of blocks
		vmarker = vcount;
		for (int j = 0; j < m_M; j++)
		{
			for (int k = 0; k < m_M; k++)
			{
				vec2 v;
				vec2 tc;

				v.x = left * quad_size + (k + m_M - 1) * quad_size;
				v.y = ffar * quad_size + (j + m_N - m_M) * quad_size;
				tc.x = 0.5f + left * texel_size + (k + m_M - 1) * texel_size;
				tc.y = 0.5f + ffar * texel_size + (j + m_N - m_M) * texel_size;

				vertices.push_back(v);
				texcoords.push_back(tc);
				vcount++;
			}
		}
		create_block(vmarker, m_M, m_M, vertices, cullable);
		vmarker = vcount;
		for (int j = 0; j < m_M; j++)
		{
			for (int k = 0; k < 3; k++)
			{
				vec2 v;
				vec2 tc;

				v.x = left * quad_size + (k + 2 * m_M - 2) * quad_size;
				v.y = ffar * quad_size + (j + m_N - m_M) * quad_size;
				tc.x = 0.5f + left * texel_size + (k + 2 * m_M - 2) * texel_size;
				tc.y = 0.5f + ffar * texel_size + (j + m_N - m_M) * texel_size;

				vertices.push_back(v);
				texcoords.push_back(tc);
				vcount++;
			}
		}
		create_block(vmarker, 3, m_M, vertices, cullable);
		vmarker = vcount;
		for (int j = 0; j < m_M; j++)
		{
			for (int k = 0; k < m_M; k++)
			{
				vec2 v;
				vec2 tc;

				v.x = left * quad_size + (k + 2 * m_M) * quad_size;
				v.y = ffar * quad_size + (j + m_N - m_M) * quad_size;
				tc.x = 0.5f + left * texel_size + (k + 2 * m_M) * texel_size;
				tc.y = 0.5f + ffar * texel_size + (j + m_N - m_M) * texel_size;

				vertices.push_back(v);
				texcoords.push_back(tc);
				vcount++;
			}
		}
		create_block(vmarker, m_M, m_M, vertices, cullable);
	}

	m_min_draw_count = indices.size();
	// Add this offset to all blocks' start indices
	for (int i = 0; i < (int)blocks.size(); i++)
		blocks[i].start_index += m_min_draw_count * sizeof(GLuint);
	// Append the cullable block indices to the base indices
	indices.insert(indices.end(), cullable.begin(), cullable.end());

	// Add the base indices to the draw list
	m_draw_count[0] = m_min_draw_count;
	m_draw_starts[0] = 0;

	roy_log(LOG_INFO, "Furthest point:%d m", -ffar * quad_size);
	roy_log(LOG_INFO, "Vertex count:%d", (int)vertices.size());
	roy_log(LOG_INFO, "Index count:%d", (int)indices.size());
	roy_log(LOG_INFO, "Clipmap Memory:%d MiB", (texcoords.size() * 2 + vertices.size() * 3 + indices.size()) * 4 / 1024.0f / 1024.0f);

	// OpenGL STUFF
	/////////////////////////////////////

	// Create the vertex array
	glGenVertexArrays(2, m_vao);

	// Generate three VBOs for vertices, texture coordinates and indices
	glGenBuffers(6, m_vbo);

	// OUTER LEVELS
	glBindVertexArray(m_vao[0]);
	// Setup the vertex buffer
	glBindBuffer(GL_ARRAY_BUFFER, m_vbo[0]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vec2) * vertices.size(), &vertices[0], GL_STATIC_DRAW);
	glVertexAttribPointer((GLuint)0, 2, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(0);
	// Setup the texcoord buffer
	glBindBuffer(GL_ARRAY_BUFFER, m_vbo[1]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vec2) * vertices.size(), &texcoords[0], GL_STATIC_DRAW);
	glVertexAttribPointer((GLuint)1, 2, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(1);
	// Setup the index buffer
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_vbo[2]);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(GLuint) * indices.size(), &indices[0], GL_STATIC_DRAW);

	// INNER GRID
	glBindVertexArray(m_vao[1]);
	// Setup the vertex buffer
	glBindBuffer(GL_ARRAY_BUFFER, m_vbo[3]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vec2) * vertices_inner.size(), &vertices_inner[0], GL_STATIC_DRAW);
	glVertexAttribPointer((GLuint)0, 2, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(0);
	// Setup the texcoord buffer
	glBindBuffer(GL_ARRAY_BUFFER, m_vbo[4]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vec2) * vertices_inner.size(), &texcoords_inner[0], GL_STATIC_DRAW);
	glVertexAttribPointer((GLuint)1, 2, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(1);
	// Setup the index buffer
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_vbo[5]);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(GLushort) * indices_inner.size(), &indices_inner[0], GL_STATIC_DRAW);
	m_nInnerIndices = indices_inner.size();
}

void create_block(struct clipmap* cmap, int vertstart, int width, int height, std::vector<vec2>& vertices, std::vector<GLuint>& indices) {
	cull_block block;
	int idx;

	idx = indices.size();
	block.start_index = indices.size() * sizeof(GLuint);	// offset into indexbuffer
	block.count = indices.size(); // number of indices

	// Form the triangles and push them onto the index list
	for (int y = 0; y < height - 1; y++)
	{
		for (int x = 0; x < width - 1; x++)
		{
			if (x == 0)
			{
				indices.push_back((y + 0) * width + (x + 0) + vertstart);
				indices.push_back((y + 1) * width + (x + 0) + vertstart);
			}
			indices.push_back((y + 0) * width + (x + 1) + vertstart);
			indices.push_back((y + 1) * width + (x + 1) + vertstart);
		}
		// degens to start new row
		if (y < height - 2)
		{
			indices.push_back((y + 1) * width + (width - 1) + vertstart);
			indices.push_back((y + 1) * width + (0) + vertstart);
		}
	}
	block.count = indices.size() - block.count;


	int row = (width - 1) * 2 + 2;
	block.bound[0] = vertices[indices[idx]];
	block.bound[1] = vertices[indices[idx + row - 2]];
	block.bound[2] = vertices[indices[idx + block.count + 1 - row]];
	block.bound[3] = vertices[indices[idx + block.count - 1]];

	blocks.push_back(block);
}

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

#endif
