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

void Texture2D::UpdateTexture2D(int width, int height, int linesize, const void* data)
{
	glBindTexture(GL_TEXTURE_2D, texID);
	glPixelStorei(GL_UNPACK_ROW_LENGTH, linesize);
	glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, width, height, format, GL_UNSIGNED_BYTE, data);
	glBindTexture(GL_TEXTURE_2D, 0);
}
