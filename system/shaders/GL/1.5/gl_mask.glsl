#version 150

uniform sampler2D img;
uniform sampler2D imgMask;
uniform float m_alpha;
in vec2 m_cord;
out vec4 fragColor;


void main()
{
  vec3 video = texture(img, m_cord).rgb;
  vec3 mask = texture(imgMask, m_cord).rgb;
  vec3 tmp = mask;
  
  if(video.r < mask.r)
    tmp.r = video.r;
  if(video.g < mask.g)
    tmp.g = video.g;
  if(video.b < mask.b)
    tmp.b = video.b;
    
  fragColor.rgb = tmp;
  fragColor.a = m_alpha;
}
