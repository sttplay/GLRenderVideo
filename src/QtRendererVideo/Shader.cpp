#include "Shader.h"
#include "Texture2D.h"
Shader::Shader(const char* vsUrl, const char* fsUrl)
{
	program = CreateGPUProgram(vsUrl, fsUrl);

	posLocation = glGetAttribLocation(program, "pos");
	colorLocation = glGetAttribLocation(program, "color");
	texcoordLocation = glGetAttribLocation(program, "texcoord");

	/*smp1 = glGetUniformLocation(program, "smp1");
	smp2 = glGetUniformLocation(program, "smp2");*/

	modelLocation = glGetUniformLocation(program, "modelMat");
	viewLocation = glGetUniformLocation(program, "viewMat");
	projLocation = glGetUniformLocation(program, "projMat");
}

Shader::~Shader()
{
	glDeleteProgram(program);
}

void Shader::Apply()
{
	memset(textureUnit, 0, sizeof(textureUnit));
	glUseProgram(program);
}

void Shader::SetTexture2D(const char* locationName, Texture2D *texture)
{
	int unit = -1;
	for (int i = 0; i < MAX_TEXTURE_UNIT; i++)
	{
		if (!textureUnit[i])
		{
			unit = i;
			textureUnit[unit] = true;
			break;
		}
	}
	if (unit < 0)
		throw;

	GLenum textureID = GL_TEXTURE0 + unit;
	glActiveTexture(textureID);
	glBindTexture(GL_TEXTURE_2D, texture->texID);
	GLint location = glGetUniformLocation(program, locationName);
	if (location < 0)
		throw;
	glUniform1i(location, unit);
}
