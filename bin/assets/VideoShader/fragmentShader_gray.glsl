
varying vec3 m_color;
varying vec2 m_texcoord;

uniform sampler2D smp1;

void main()
{

	vec4 normalColor = vec4(m_color, 1.0) * 0.0001;

	vec3 rgb = texture2D(smp1, m_texcoord).rgb;
	gl_FragColor = vec4(rgb, 1.0) + normalColor;
}