#version 150

uniform sampler2D img;
uniform float m_alpha;
in vec2 m_cord;
out vec4 fragColor;


void main()
{
  fragColor.rgb = texture(img, m_cord).rgb;
  fragColor.r = 1.0;
  fragColor.a = m_alpha;
}
