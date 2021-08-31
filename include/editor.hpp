/*
* Editor and Debug drawing functions
*
*/



// Bullet Physics Debug interface
#ifdef __BULLET__

#ifndef DEBUG_RENDERER_HPP
#define DEBUG_RENDERER_HPP

#pragma once

#include "btIDebugDraw.h"
#include "shader.h"
#include "opengl.h"
#include "linalg.h"
#include "util.h"

class btDebugDraw : public btIDebugDraw {
	int m_debugMode;
	// OpenGL 4 
	
	struct shader_t debug_shader;
	int color_uniform_loc;
	int mvp_uniform_loc;

	float line_vert[6] = {
		// x     y     z
		 0.0f,  0.0f,  0.0f,
		 0.0f,  0.0f,  0.0f,
	};
	buffer_t* line_vert_buf;
	float tri_vert[9] = {
		// x     y     z
		 0.5f,  0.0f,  0.5f,
		-0.5f,  0.0f,  0.5f,
		 0.0f,  0.0f,  0.5f,
	};
	buffer_t* tri_vert_buf;
	GLuint debug_VAO;
	GLuint debug_triVAO;
public:
	float mvp[16];

	btDebugDraw::btDebugDraw():m_debugMode(0) { 
		char* debug_vert_shader = ""
			"layout (location = 0) in vec3 aPosition; \n"
			"out vec3 vertex_color;\n"
			"uniform mat4 mvp;\n"
			"void main() {\n"
			"	gl_Position = mvp * vec4(aPosition,1.0);\n"
			"}\n\0";

		char* debug_frag_shader = ""
			"out vec4 FragColor;\n"
			"uniform vec3 color;\n"
			"void main() {\n"
			"	FragColor = vec4(color,1.0);\n"
			"}\n\0";

		shader_graphics_new(
			&this->debug_shader,
			debug_vert_shader, 143,
			debug_frag_shader, 88
		);

		mat4_identity(mvp);

		gl_use_program(debug_shader.program);
		this->color_uniform_loc = glGetUniformLocation(debug_shader.program, "color");
		glUniform3f(this->color_uniform_loc, 1., .0, .0);
		this->mvp_uniform_loc = glGetUniformLocation(debug_shader.program, "mvp");


		glGenVertexArrays(1, &this->debug_VAO);
		glBindVertexArray(this->debug_VAO);

		this->line_vert_buf = buffer_new(sizeof(line_vert), line_vert, BUFFER_VERTEX, USAGE_DYNAMIC, false);
		gl_gpu_bind_buffer(line_vert_buf->type, line_vert_buf->id);

		// vertex position attribute
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
		glEnableVertexAttribArray(0);
		glBindVertexArray(0);

		glGenVertexArrays(1, &this->debug_triVAO);
		glBindVertexArray(this->debug_triVAO);

		this->tri_vert_buf = buffer_new(sizeof(tri_vert), tri_vert, BUFFER_VERTEX, USAGE_DYNAMIC, false);
		gl_gpu_bind_buffer(tri_vert_buf->type, tri_vert_buf->id);

		// vertex position attribute
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
		glEnableVertexAttribArray(0);
		glBindVertexArray(0);
	
	}

	virtual ~btDebugDraw() {}

	virtual void btDebugDraw::drawLine(const btVector3& from, const btVector3& to, const btVector3& fromColor, const btVector3& toColor) {
		// update vertex positions
		float new_line_vert[6] = {
			 from.x(), from.y(), from.z(),
			 to.x(),	to.y(),		to.z(),
		};
		
		gl_use_program(debug_shader.program);
		glUniform3f(this->color_uniform_loc, (GLfloat)fromColor.x(), (GLfloat)fromColor.y(), (GLfloat)fromColor.z());
		glUniformMatrix4fv(this->mvp_uniform_loc, 1, GL_FALSE, (GLfloat*)this->mvp);
		glBindVertexArray(this->debug_VAO);
		glNamedBufferSubData(line_vert_buf->id, 0, sizeof(new_line_vert), (void*)new_line_vert);
		gl_gpu_bind_buffer(line_vert_buf->type, line_vert_buf->id);
		glDrawArrays(GL_LINES, 0, 2);
		glBindVertexArray(0);
		gl_use_program(0);
		
		/*
		glBegin(GL_LINES);
		glColor3f(fromColor.getX(), fromColor.getY(), fromColor.getZ());
		glVertex3d(from.getX(), from.getY(), from.getZ());
		glColor3f(toColor.getX(), toColor.getY(), toColor.getZ());
		glVertex3d(to.getX(), to.getY(), to.getZ());
		glEnd();
		*/
	}

	virtual void btDebugDraw::drawLine(const btVector3& from, const btVector3& to, const btVector3& color) {
		drawLine(from, to, color, color);
	}

	virtual void btDebugDraw::drawSphere(const btVector3& p, btScalar radius, const btVector3& color) {
		glColor4f(color.getX(), color.getY(), color.getZ(), btScalar(1.0f));
		glPushMatrix();
		glTranslatef(p.getX(), p.getY(), p.getZ());

		int lats = 5;
		int longs = 5;

		int i, j;
		for (i = 0; i <= lats; i++) {
			btScalar lat0 = SIMD_PI * (-btScalar(0.5) + (btScalar)(i - 1) / lats);
			btScalar z0 = radius * sin(lat0);
			btScalar zr0 = radius * cos(lat0);

			btScalar lat1 = SIMD_PI * (-btScalar(0.5) + (btScalar)i / lats);
			btScalar z1 = radius * sin(lat1);
			btScalar zr1 = radius * cos(lat1);

			glBegin(GL_QUAD_STRIP);
			for (j = 0; j <= longs; j++) {
				btScalar lng = 2 * SIMD_PI * (btScalar)(j - 1) / longs;
				btScalar x = cos(lng);
				btScalar y = sin(lng);

				glNormal3f(x * zr0, y * zr0, z0);
				glVertex3f(x * zr0, y * zr0, z0);
				glNormal3f(x * zr1, y * zr1, z1);
				glVertex3f(x * zr1, y * zr1, z1);
			}
			glEnd();
		}

		glPopMatrix();
	}

	virtual void btDebugDraw::drawTriangle(const btVector3& a, const btVector3& b, const btVector3& c, const btVector3& color, btScalar alpha) {
		// update vertex positions
		float new_tri_vert[9] = {
			// x     y     z
			 a.x(), a.y(), a.z(),
			 b.x(), b.y(), b.z(),
			 c.x(), c.y(), c.z(),
		};

		gl_use_program(debug_shader.program);
		glUniform3f(this->color_uniform_loc, (GLfloat)color.x(), (GLfloat)color.y(), (GLfloat)color.z());
		glUniformMatrix4fv(this->mvp_uniform_loc, 1, GL_FALSE, (GLfloat*)this->mvp);
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		glBindVertexArray(this->debug_triVAO);
		glNamedBufferSubData(tri_vert_buf->id, 0, sizeof(new_tri_vert), (void*)new_tri_vert);
		gl_gpu_bind_buffer(tri_vert_buf->type, tri_vert_buf->id);
		glDrawArrays(GL_TRIANGLES, 0, 3);
		glBindVertexArray(0);
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		gl_use_program(0);

		/*
		const btVector3	n = btCross(b - a, c - a).normalized();
		glBegin(GL_TRIANGLES);
		glColor4f(color.getX(), color.getY(), color.getZ(), alpha);
		glNormal3d(n.getX(), n.getY(), n.getZ());
		glVertex3d(a.getX(), a.getY(), a.getZ());
		glVertex3d(b.getX(), b.getY(), b.getZ());
		glVertex3d(c.getX(), c.getY(), c.getZ());
		glEnd();
		*/
	}

	virtual void btDebugDraw::drawContactPoint(const btVector3& pointOnB, const btVector3& normalOnB, btScalar distance, int lifeTime, const btVector3& color) {

		{
			btVector3 to = pointOnB + normalOnB * 1;//distance;
			const btVector3&from = pointOnB;
			glColor4f(color.getX(), color.getY(), color.getZ(), 1.f);
			//glColor4f(0,0,0,1.f);
			glBegin(GL_LINES);
			glVertex3d(from.getX(), from.getY(), from.getZ());
			glVertex3d(to.getX(), to.getY(), to.getZ());
			glEnd();


			//		glRasterPos3f(from.x(),  from.y(),  from.z());
			//		char buf[12];
			//		sprintf(buf," %d",lifeTime);
					//BMF_DrawString(BMF_GetFont(BMF_kHelvetica10),buf);


		}
	}

	virtual void btDebugDraw::reportErrorWarning(const char* warningString) {
		roy_log(LOG_WARN, "BULLET", "%s", warningString);
	}


	virtual void btDebugDraw::draw3dText(const btVector3& location, const char* textString) {
		glRasterPos3f(location.x(), location.y(), location.z());
		//BMF_DrawString(BMF_GetFont(BMF_kHelvetica10),textString);
	}

	virtual void btDebugDraw::setDebugMode(int debugMode) {
		m_debugMode = debugMode;
	}

	virtual int		getDebugMode() const { return m_debugMode; }

};

#endif
#endif












