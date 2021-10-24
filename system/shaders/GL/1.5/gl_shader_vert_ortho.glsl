#version 150

in vec2 m_attrpos;
in vec4 m_attrcol;
in vec2 m_attrcord0;
in vec2 m_attrcord1;
out vec4 m_cord0;
out vec4 m_cord1;
out vec4 m_colour;
uniform vec2 m_size;
uniform vec2 m_position;

void main ()
{
  vec2 pos = m_size * m_attrpos + m_position;
  // vec2 pos = 100.5 * m_attrpos.xy + 0.;
  gl_Position = vec4(pos*(1920.),28.,1920.);
  m_colour    = m_attrcol;
  //vec2 m_attrcord0 = vec2(.5);
  m_cord0     = vec4(m_attrcord0, 0., 0.);
  m_cord1     = vec4(m_attrcord1, 0., 0.);
}
