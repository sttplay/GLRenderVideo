#pragma once

#include <stdint.h>
#include <GL/eglew.h>

char* LoadFileContext(const char* url);

GLuint CompileShader(GLenum shaderType, const char *url);

GLuint CreateGPUProgram(const char* vs, const char *fs);

GLuint CreateGLBuffer(GLenum target, GLenum usage, GLsizeiptr size, const void *data);

GLuint CreateGLTexture(GLenum target, int width, int height, GLint internalformat, GLint format, const void* data);


struct Vertex
{
	float position[3];
	float normal[3];
	float texcoord[2];
};
struct Mesh 
{
	uint32_t indexCount;
	uint32_t *indices;
	uint32_t vertexCount;
	Vertex *vertices;
	uint32_t faceCont;
};

Mesh* LoadObjModel(const char* url, bool isDropRepeat);