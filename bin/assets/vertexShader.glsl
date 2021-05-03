attribute vec3 pos;
attribute vec3 color;
varying vec3 m_color;
void main()
{
	m_color = color;
	gl_Position = vec4(pos, 1.0f);
}