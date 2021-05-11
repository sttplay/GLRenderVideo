#include "Texture2D.h"

Texture2D::Texture2D(int width, int height, GLint internalFormat, GLint format, const void *data)
{
	this->width = width;
	this->height = height;
	this->internalFormat = internalFormat;
	this->format = format;
	texID = CreateGLTexture(GL_TEXTURE_2D, width, height, internalFormat, format, data);
}

Texture2D::~Texture2D()
{
	glDeleteTextures(1, &texID);
}
