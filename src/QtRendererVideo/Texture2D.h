#pragma once
#include "GLTools.h"
class Texture2D
{
public:
	Texture2D(int width, int height, GLint internalFormat, GLint format, const void *data);
	~Texture2D();

public:
	int width;
	int height;
	GLint internalFormat;
	GLint format;

	GLuint texID;
};

