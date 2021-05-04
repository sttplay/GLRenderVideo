attribute vec3 pos;
attribute vec3 color;
attribute vec2 texcoord;
varying vec3 m_color;
varying vec2 m_texcoord;
void main()
{
	m_color = color;
	m_texcoord = texcoord;
	m_texcoord.y *= -1;
	gl_Position = vec4(pos, 1.0f);
}