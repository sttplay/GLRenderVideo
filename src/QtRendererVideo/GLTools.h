#pragma once

#include <stdint.h>
#include <GL/eglew.h>

char* LoadFileContext(const char* url);

GLuint CompileShader(GLenum shaderType, const char *url);

GLuint CreateGPUProgram(const char* vs, const char *fs);

GLuint CreateGLBuffer(GLenum target, GLenum usage, GLsizeiptr size, const void *data);

GLuint CreateGLTexture(GLenum target, int width, int height, GLint internalformat, GLint format, const void* data);