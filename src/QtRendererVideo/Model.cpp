#include "Model.h"
#include "Shader.h"
#include <QMatrix4x4>
Model::Model(const char *url)
{
	mesh = LoadObjModel(url, true);
	VBO = CreateGLBuffer(GL_ARRAY_BUFFER, GL_STATIC_DRAW, mesh->vertexCount * sizeof(Vertex), mesh->vertices);

	EBO = CreateGLBuffer(GL_ELEMENT_ARRAY_BUFFER, GL_STATIC_DRAW, mesh->indexCount * sizeof(uint32_t), mesh->indices);

	position = QVector3D(0, 0, 0);
	angle = 0;
	axis = QVector3D(0, 1, 0);
	scale = QVector3D(1, 1, 1);
}

Model::~Model()
{
	if (mesh)
	{
		delete mesh->indices;
		delete mesh->vertices;
		delete mesh;
	}
}

void Model::ApplyShader(Shader* shader)
{
	this->shader = shader;
	shader->Apply();
}

void Model::SetTexture2D(const char* locationName, Texture2D *texture)
{
	shader->SetTexture2D(locationName, texture);
}

void Model::SetPosition(float x, float y, float z)
{
	position = QVector3D(x, y, z);
}

void Model::SetRotation(float angle, float axisX, float axisY, float axisZ)
{
	this->angle = angle;
	axis = QVector3D(axisX, axisY, axisZ);
}

void Model::SetScale(float x, float y, float z)
{
	scale = QVector3D(x, y, z);
}

void Model::Draw(const float *viewMat, const float *projMat)
{
	glBindBuffer(GL_ARRAY_BUFFER, VBO);

	//启用顶点属性
	glEnableVertexAttribArray(shader->posLocation);
	glVertexAttribPointer(
		shader->posLocation, //顶点属性ID
		3, //几个数据构成一组
		GL_FLOAT, //数据类型
		GL_FALSE,
		sizeof(float) * 8, //步长
		(void*)(sizeof(float) * 0) //偏移量，第一组数据的起始位置
	);
	//启用顶点属性
	glEnableVertexAttribArray(shader->colorLocation);
	glVertexAttribPointer(shader->colorLocation, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 8, (void*)(sizeof(float) * 3));

	//启用顶点属性
	glEnableVertexAttribArray(shader->texcoordLocation);
	glVertexAttribPointer(shader->texcoordLocation, 2, GL_FLOAT, GL_FALSE, sizeof(float) * 8, (void*)(sizeof(float) * 6));

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

	QMatrix4x4 modelMat;
	modelMat.translate(position);
	modelMat.rotate(angle, axis);
	modelMat.scale(scale);

	glUniformMatrix4fv(shader->modelLocation, 1, GL_FALSE, modelMat.constData());
	glUniformMatrix4fv(shader->viewLocation, 1, GL_FALSE, viewMat);
	glUniformMatrix4fv(shader->projLocation, 1, GL_FALSE, projMat);
	//glBindVertexArray(VAO);
	//glDrawArrays(GL_TRIANGLES, 0, mesh->vertexCount);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	glDrawElements(GL_TRIANGLES, mesh->indexCount, GL_UNSIGNED_INT, NULL);
}
