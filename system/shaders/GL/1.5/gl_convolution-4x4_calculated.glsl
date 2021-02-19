#version 150

uniform sampler2D img;
uniform vec2 stepxy;
uniform float m_stretch;
uniform float m_alpha;
in vec2 m_cord;
out vec4 fragColor;


#if (XBMC_SCALER_WEIGHT_BSPLINE)
vec4 weight(float pos)
{
  vec2 posAD = vec2(2.-pos, 1.+pos);
  vec2 posBC = vec2(1.-pos, pos);
  vec4 result;
  result.xw = -(posAD * posAD * posAD) / 6. + posAD * posAD - 2. * posAD + 4. / 3.;
  result.yz = 0.5 * posBC * posBC * posBC - posBC * posBC + 2. / 3.;
  return result;
}

#elif (XBMC_SCALER_WEIGHT_BICUBIC)
vec4 weight(float pos)
{
  vec2 posAD = vec2(2. - pos, 1. + pos);
  vec2 posBC = vec2(1. - pos, pos);
  vec4 result;

  result.xw = 1. / 6. * ((-constantB - 6. * constantC) * posAD * posAD * posAD + 
                         (6. * constantB + 30. * constantC) * posAD * posAD +
                         (-12. * constantB - 48. * constantC) * posAD +
                         (8. * constantB + 24. * constantC));
  
  
  
  result.yz = 1. / 6. * ((12. - 9. * constantB - 6. * constantC) * posBC * posBC * posBC +
                         (-18. + 12. * constantB + 6. * constantC) * posBC * posBC +
                         (6. - 2. + constantB));
  return result;
}

#elif (XBMC_SCALER_WEIGHT_LANCZOS)
vec4 weight(float pos)
{
  vec4 x = vec4(2. - pos, 1. - pos, pos, 1. + pos);
  vec4 result;
  float pi = 3.1415926535;
  float a = 2.;
  result = (a*sin(pi*x) * sin(0.5*(pi*x)))/(pi*pi*x*x);
  result = min(vec4(1.),result);
  return result;
}
#endif

vec2 stretch(vec2 pos)
{
#if (XBMC_STRETCH)
  // our transform should map [0..1] to itself, with f(0) = 0, f(1) = 1, f(0.5) = 0.5, and f'(0.5) = b.
  // a simple curve to do this is g(x) = b(x-0.5) + (1-b)2^(n-1)(x-0.5)^n + 0.5
  // where the power preserves sign. n = 2 is the simplest non-linear case (required when b != 1)
  float x = pos.x - 0.5;
  return vec2(mix(x * abs(x) * 2.0, x, m_stretch) + 0.5, pos.y);
#else
  return pos;
#endif
}

vec3 pixel(float xpos, float ypos)
{
  return texture(img, vec2(xpos, ypos)).rgb;
}

vec3 line (float ypos, vec4 xpos, vec4 linetaps)
{
  return
    pixel(xpos.r, ypos) * linetaps.r +
    pixel(xpos.g, ypos) * linetaps.g +
    pixel(xpos.b, ypos) * linetaps.b +
    pixel(xpos.a, ypos) * linetaps.a;
}

vec4 process()
{
  vec4 rgb;
  vec2 pos = stretch(m_cord) + stepxy * 0.5;
  vec2 f = fract(pos / stepxy);

  vec4 linetaps   = weight(1.0 - f.x);
  vec4 columntaps = weight(1.0 - f.y);

  vec2 xystart = (-1.5 - f) * stepxy + pos;
  vec4 xpos = vec4(xystart.x, xystart.x + stepxy.x, xystart.x + stepxy.x * 2.0, xystart.x + stepxy.x * 3.0);

  rgb.rgb =
    line(xystart.y                 , xpos, linetaps) * columntaps.r +
    line(xystart.y + stepxy.y      , xpos, linetaps) * columntaps.g +
    line(xystart.y + stepxy.y * 2.0, xpos, linetaps) * columntaps.b +
    line(xystart.y + stepxy.y * 3.0, xpos, linetaps) * columntaps.a;

  rgb.a = m_alpha;
  return rgb;
}
