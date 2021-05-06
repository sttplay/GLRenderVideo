#pragma once
#include <QMatrix4x4>
class Camera
{
public:
	Camera();
	~Camera();

	void SetPosition(float x, float y, float z);

	void SetRotation(float pitch, float yaw, float roll = 0);

	void Translate(float x, float y, float z);

	void Rotate(float pitch, float yaw, float roll);

	QMatrix4x4 GetViewMat();

private:

	QVector3D up;
	QVector3D position;
	QVector3D rotation;
	QVector3D forward;
};

