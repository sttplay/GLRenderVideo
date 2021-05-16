
varying vec3 m_color;
varying vec2 m_texcoord;

uniform sampler2D smp1;
uniform sampler2D smp2;

//BT.601
const vec3 offset = vec3(-0.0627451017, -0.501960814, -0.501960814);
const vec3 Rcoeff = vec3(1.1644, 0.000, 1.596);						
const vec3 Gcoeff = vec3(1.1644, -0.3918, -0.813);					
const vec3 Bcoeff = vec3(1.1644, 2.0172, 0.000);

void main()
{

	vec4 normalColor = vec4(m_color, 1.0) * 0.0001;

	vec3 yuv;
	yuv.x = texture2D(smp1, m_texcoord).r;
	yuv.y = texture2D(smp2, m_texcoord).g;
	yuv.z = texture2D(smp2, m_texcoord).a;
	yuv += offset;
	vec3 rgb;
	rgb.r = dot(yuv, Rcoeff);
	rgb.g = dot(yuv, Gcoeff);
	rgb.b = dot(yuv, Bcoeff);
	gl_FragColor = vec4(rgb, 1.0) + normalColor;
}