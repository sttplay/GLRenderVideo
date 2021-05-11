#pragma once
#include "GLTools.h"
class Texture2D;
#define  MAX_TEXTURE_UNIT 32
class Shader
{
public:
	Shader(const char* vsUrl, const char* fsUrl);
	~Shader();

	void Apply();

	void SetTexture2D(const char* locationName, Texture2D *texture);
public:
	GLint posLocation, colorLocation, texcoordLocation;
	GLint modelLocation, viewLocation, projLocation;

private:
	GLuint program;

	bool textureUnit[MAX_TEXTURE_UNIT];
};

