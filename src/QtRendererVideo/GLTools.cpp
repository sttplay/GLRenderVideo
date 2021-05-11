#include "GLTools.h"
#include <iostream>
#include <sstream>
#include <vector>
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
		1, //创建缓冲的个数
		&bufferID
	);
	glBindBuffer(target, bufferID);
	glBufferData(
		target, //缓冲类型
		size, //数据量的大小
		data,//顶点数据
		usage
	);
	//GL_STATIC_DRAW//数据几乎不会改变
	//GL_DYNAMIC_DRAW 数据可能会发生改变
	//GL_STREAM_DRAW 每次绘制数据都会发生改变
	glBindBuffer(target, 0);
	return bufferID;
}

GLuint CreateGLTexture(GLenum target, int width, int height, GLint internalformat, GLint format, const void* data)
{
	GLuint texID;
	glGenTextures(1, &texID);
	glBindTexture(target, texID);
	//设置环绕方式
	glTexParameteri(target, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(target, GL_TEXTURE_WRAP_T, GL_REPEAT);
	//设置过滤方式
	glTexParameteri(target, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(target, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	glTexImage2D(
		target, //纹理类型
		0, //多级渐远纹理级别
		internalformat, //纹理的格式
		width,
		height,
		0,
		format, //数据的格式
		GL_UNSIGNED_BYTE,
		data
	);
	glGenerateMipmap(target);
	glBindTexture(target, 0);
	return texID;
}

struct VertexAttri
{
	float x;
	float y;
	float z;
};

struct VertexIndex
{
	int posIndex;
	int texIndex;
	int norIndex;
};
Mesh* LoadObjModel(const char* url, bool isDropRepeat)
{
	printf("===================================\n");
	char * fileContext = LoadFileContext(url);
	if (!fileContext)
		return NULL;

	std::vector<VertexAttri> positions, normals, texcoords;
	std::vector<VertexIndex> vertices;
	std::vector<uint32_t> indices;
	std::stringstream strStream(fileContext);
	delete fileContext;

	std::string temp;
	while (!strStream.eof())
	{
		const int MAX_SIZE = 256;
		char lineStr[MAX_SIZE];
		strStream.getline(lineStr, MAX_SIZE);

		std::stringstream lineStream(lineStr);
		if (lineStr[0] == 'v')
		{
			if (lineStr[1] == 't')
			{
				lineStream >> temp;
				VertexAttri attri;
				lineStream >> attri.x;
				lineStream >> attri.y;
				lineStream >> attri.z;
				texcoords.push_back(attri);
			}
			else if (lineStr[1] == 'n')
			{
				lineStream >> temp;
				VertexAttri attri;
				lineStream >> attri.x;
				lineStream >> attri.y;
				lineStream >> attri.z;
				normals.push_back(attri);
			}
			else
			{
				lineStream >> temp;
				VertexAttri attri;
				lineStream >> attri.x;
				lineStream >> attri.y;
				lineStream >> attri.z;
				positions.push_back(attri);
			}
		}
		else if (lineStr[0] == 'f')
		{
			lineStream >> temp;
			for (int i = 0; i < 3; i++)
			{
				lineStream >> temp;
				int pos1 = temp.find_first_of('/');
				std::string vStr = temp.substr(0, pos1);
				int pos2 = temp.find_first_of('/', pos1 + 1);
				std::string tStr = temp.substr(pos1 + 1, pos2 - pos1 - 1);
				std::string nStr = temp.substr(pos2 + 1, temp.length() - pos2 - 1);

				VertexIndex vi;
				//顶点索引信息，
				//顶点对应着顶点位置,法线和纹理坐标
				//该结构体记录着当前点所对应各自的属性容器索引值
				//posIndex --> positions 向量容器中的位置索引
				//texIndex --> texcoords 向量容器中的位置索引
				//norIndex --> normals	 向量容器中的位置索引
				vi.posIndex = atoi(vStr.c_str()) - 1;
				vi.texIndex = atoi(tStr.c_str()) - 1;
				vi.norIndex = atoi(nStr.c_str()) - 1;

				int index = -1;
				//重复检查
				if (isDropRepeat)
				{
					int currentVertexCount = vertices.size();
					for (int k = 0; k < currentVertexCount; k++)
					{
						if (vertices[k].posIndex == vi.posIndex &&
							vertices[k].norIndex == vi.norIndex &&
							vertices[k].texIndex == vi.texIndex)
						{
							index = k;
							break;
						}
					}
				}
				if (index == -1)
				{
					index = vertices.size();
					vertices.push_back(vi);
				}
				indices.push_back(index);
			}
		}
	}
	Mesh *mesh = new Mesh();
	mesh->indexCount = indices.size();
	mesh->indices = new uint32_t[mesh->indexCount];
	memcpy(mesh->indices, &indices[0], mesh->indexCount * sizeof(uint32_t));

	mesh->vertexCount = vertices.size();
	mesh->vertices = new Vertex[mesh->vertexCount];
	for (int i = 0; i < mesh->vertexCount; i++)
	{
		//从当前顶点索引容器中取出相应的数据拷贝到Vertex结构体中的对应位置
		memcpy(&mesh->vertices[i].position, &positions[vertices[i].posIndex], sizeof(float) * 3);
		memcpy(&mesh->vertices[i].normal, &normals[vertices[i].norIndex], sizeof(float) * 3);
		memcpy(&mesh->vertices[i].texcoord, &texcoords[vertices[i].texIndex], sizeof(float) * 2);
	}
	mesh->faceCont = mesh->indexCount / 3;
	return mesh;
}

void CheckError()
{
	GLenum errorCode = glGetError();
	if (errorCode)
	{
		printf("glGetError:%d\n", errorCode);
		throw;
	}
}

