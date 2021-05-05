#include "GLTools.h"
#include <iostream>
char* LoadFileContext(const char* url)
{
	char* fileContext = NULL;
	FILE *fp = fopen(url, "rb");
	if (fp)
	{
		fseek(fp, 0, SEEK_END);
		int len = ftell(fp);
		if (len > 0)
		{
			rewind(fp);
			fileContext = new char[len + 1];
			fread(fileContext, 1, len, fp);
			fileContext[len] = 0;
		}
		fclose(fp);
	}
	return fileContext;
}


GLuint CompileShader(GLenum shaderType, const char *url)
{
	char *shaderCode = LoadFileContext(url);
	char *shaderTypeStr = "Vertex Shader";
	if (shaderType == GL_FRAGMENT_SHADER)
	{
		shaderTypeStr = "Fragment Shader";
	}
	GLuint shader = glCreateShader(shaderType);
	if (!shader)
		throw;
	glShaderSource(shader, 1, &shaderCode, NULL);
	glCompileShader(shader);
	GLint success = GL_TRUE;
	glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
	if (!success)
	{
		char infolog[1024];
		GLsizei logLen = 0;
		glGetShaderInfoLog(shader, sizeof(infolog), &logLen, infolog);
		printf("[ERROR] Compile %s error: %s", shaderTypeStr, infolog);
		glDeleteShader(shader);
		throw;
	}
	printf("Compile Success");
	delete shaderCode;
	return shader;
}

GLuint CreateGPUProgram(const char* vs, const char *fs)
{
	GLuint vsShader = CompileShader(GL_VERTEX_SHADER, vs);
	GLuint fsShader = CompileShader(GL_FRAGMENT_SHADER, fs);

	GLuint shaderProgram = glCreateProgram();
	glAttachShader(shaderProgram, vsShader);
	glAttachShader(shaderProgram, fsShader);
	glLinkProgram(shaderProgram);

	GLint success = GL_TRUE;
	glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
	if (!success)
	{
		char infolog[1024];
		GLsizei logLen = 0;
		glGetShaderInfoLog(shaderProgram, sizeof(infolog), &logLen, infolog);
		printf("[ERROR] Link error: %s", infolog);
		glDeleteProgram(shaderProgram);
		throw;
	}
	printf("Line shader program success\n");
	glDetachShader(shaderProgram, vsShader);
	glDetachShader(shaderProgram, fsShader);
	glDeleteShader(vsShader);
	glDeleteShader(fsShader);
	return shaderProgram;
}

GLuint CreateGLBuffer(GLenum target, GLenum usage, GLsizeiptr size, const void *data)
{
	GLuint bufferID;
	glGenBuffers(
		1, //��������ĸ���
		&bufferID
	);
	glBindBuffer(target, bufferID);
	glBufferData(
		target, //��������
		size, //�������Ĵ�С
		data,//��������
		usage
	);
	//GL_STATIC_DRAW//���ݼ�������ı�
	//GL_DYNAMIC_DRAW ���ݿ��ܻᷢ���ı�
	//GL_STREAM_DRAW ÿ�λ������ݶ��ᷢ���ı�
	glBindBuffer(target, 0);
	return bufferID;
}

GLuint CreateGLTexture(GLenum target, int width, int height, GLint internalformat, GLint format, const void* data)
{
	GLuint texID;
	glGenTextures(1, &texID);
	glBindTexture(target, texID);
	//���û��Ʒ�ʽ
	glTexParameteri(target, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(target, GL_TEXTURE_WRAP_T, GL_REPEAT);
	//���ù��˷�ʽ
	glTexParameteri(target, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(target, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	glTexImage2D(
		target, //��������
		0, //�༶��Զ������
		internalformat, //����ĸ�ʽ
		width,
		height,
		0,
		format, //���ݵĸ�ʽ
		GL_UNSIGNED_BYTE,
		data
	);
	glGenerateMipmap(target);
	glBindTexture(target, 0);
	return texID;
}

