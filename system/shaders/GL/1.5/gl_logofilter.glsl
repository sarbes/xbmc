#version 150

uniform sampler2D img;
uniform sampler2D mask;
uniform float m_stretch;
uniform float m_alpha;
in vec2 m_cord;
out vec4 fragColor;

void main()
{
  //fragColor = texture(img, m_cord);
  vec4 color = texture(img, m_cord);
  vec4 mask = texture(mask, m_cord);
  fragColor = (color - mask) / (1. - mask);
  //fragColor -= mask * smoothstep(0.,2.,color) * .1;
  //fragColor = (color - mask) / (1. - mask);
  //fragColor = color*(1.-mask) + (3.*color-2.)*mask;
  //fragColor = smoothstep(0.,1.,fragColor)*mask + fragColor*(1. -mask);
  fragColor = ((1.*fragColor*fragColor*fragColor - 1.*fragColor + 0.)*fragColor + fragColor)*mask*.43 + fragColor*(1.-mask*.43);
  //fragColor = texture(img, m_cord);
  //fragColor = color;
}
