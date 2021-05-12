#pragma once
#include "GLTools.h"
#include <QVector3D>
class Shader;
class Texture2D;
class Model
{
public:
	Model(const char *url);
	~Model();

	void ApplyShader(Shader* shader);

	void SetTexture2D(const char* locationName, Texture2D *texture);

	void SetPosition(float x, float y, float z);

	void SetRotation(float angle, float axisX, float axisY, float axisZ);

	void SetScale(float x, float y, float z);

	void Draw(const float *viewMat, const float *projMat);

private:

	Mesh *mesh = NULL;

	GLuint VBO, /*VAO,*/ EBO;

	QVector3D position;
	float angle;
	QVector3D axis;
	QVector3D scale;

	Shader * shader = NULL;
};

