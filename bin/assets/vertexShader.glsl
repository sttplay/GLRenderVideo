attribute vec3 pos;
attribute vec3 color;
attribute vec2 texcoord;
varying vec3 m_color;
varying vec2 m_texcoord;

uniform mat4x4 modelMat;
uniform mat4x4 viewMat;
uniform mat4x4 projMat;
void main()
{
	m_color = color;
	m_texcoord = texcoord;
	m_texcoord.y *= -1;
	gl_Position = projMat * viewMat * modelMat * vec4(pos, 1.0f);
}