#version 400

#if(XBMC_texture_rectangle)
# define texture2D texture2DRect
# define sampler2D sampler2DRect
#endif

uniform sampler2D m_sampY;
uniform sampler2D m_sampU;
uniform sampler2D m_sampV;
uniform vec2 m_step;
uniform mat4 m_yuvmat;
uniform float m_stretch;
uniform float m_alpha;
uniform sampler1D m_kernelTex;
uniform mat3 m_primMat;
uniform float m_gammaDstInv;
uniform float m_gammaSrc;
uniform float m_toneP1;
uniform vec3 m_coefsDst;
in vec2 m_cordY;
in vec2 m_cordU;
in vec2 m_cordV;
out vec4 fragColor;

vec2 stretch(vec2 pos)
{
#if (XBMC_STRETCH)
  // our transform should map [0..1] to itself, with f(0) = 0, f(1) = 1, f(0.5) = 0.5, and f'(0.5) = b.
  // a simple curve to do this is g(x) = b(x-0.5) + (1-b)2^(n-1)(x-0.5)^n + 0.5
  // where the power preserves sign. n = 2 is the simplest non-linear case (required when b != 1)
  #if(XBMC_texture_rectangle)
    float x = (pos.x * m_step.x) - 0.5;
    return vec2((mix(2.0 * x * abs(x), x, m_stretch) + 0.5) / m_step.x, pos.y);
  #else
    float x = pos.x - 0.5;
    return vec2(mix(2.0 * x * abs(x), x, m_stretch) + 0.5, pos.y);
  #endif
#else
  return pos;
#endif
}