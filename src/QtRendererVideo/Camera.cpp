#include "Camera.h"

#define PI 3.14159266
Camera::Camera()
{
	up.setX(0);
	up.setY(1);
	up.setZ(0);
	SetPosition(0, 0, 0);
	SetRotation(0, 0, 0);
}

Camera::~Camera()
{

}

void Camera::SetPosition(float x, float y, float z)
{
	position.setX(x);
	position.setY(y);
	position.setZ(z);
}

void Camera::SetRotation(float pitch, float yaw, float roll /*= 0*/)
{
	rotation.setX(pitch);
	rotation.setY(yaw);
	rotation.setZ(roll);

	//x = cos(pitch) * sin(yaw);
	//y = sin(pitch)
	//z = cos(pitch) * cos(yaw);
	forward.setX(cos(pitch * PI / 180) * sin(yaw* PI / 180));
	forward.setY(sin(pitch * PI / 180));
	forward.setZ(cos(pitch * PI / 180) * cos(yaw * PI / 180));
	forward = forward.normalized();
}

void Camera::Translate(float x, float y, float z)
{
	position += forward * z;
	position += QVector3D::crossProduct(forward, up) * x;
	position += up * y;
}

void Camera::Rotate(float pitch, float yaw, float roll)
{
	SetRotation(rotation.x() + pitch, rotation.y() + yaw, rotation.z() + roll);
}

QMatrix4x4 Camera::GetViewMat()
{
	QMatrix4x4 mat;
	mat.lookAt(position, position + forward, up);
	return mat;
}
