#version 150

uniform sampler2D img;
uniform vec2 stepxy;
uniform float m_stretch;
uniform float m_alpha;
in vec2 m_cord;
out vec4 fragColor;





float w0(float a)
{
    return (1.0/6.0)*(a*(a*(-a + 3.0) - 3.0) + 1.0);
}

float w1(float a)
{
    return (1.0/6.0)*(a*a*(3.0*a - 6.0) + 4.0);
}

float w2(float a)
{
    return (1.0/6.0)*(a*(a*(-3.0*a + 3.0) + 3.0) + 1.0);
}

float w3(float a)
{
    return (1.0/6.0)*(a*a*a);
}

// g0 and g1 are the two amplitude functions
float g0(float a)
{
    return w0(a) + w1(a);
}

float g1(float a)
{
    return w2(a) + w3(a);
}

// h0 and h1 are the two offset functions
float h0(float a)
{
    return -1.0 + w1(a) / (w0(a) + w1(a));
}

float h1(float a)
{
    return 1.0 + w3(a) / (w2(a) + w3(a));
}

vec4 texture_bicubic(sampler2D tex, vec2 uv)
{
  vec4 texelSize = vec4( stepxy,  1.0 / stepxy);
  vec2 uvorg = uv;
	uv = uv*texelSize.zw + 0.5;
	vec2 iuv = floor( uv );
	vec2 fuv = fract( uv );

  float g0x = g0(fuv.x);
  float g1x = g1(fuv.x);
  float h0x = h0(fuv.x);
  float h1x = h1(fuv.x);
  float h0y = h0(fuv.y);
  float h1y = h1(fuv.y);

	vec2 p0 = (vec2(iuv.x + h0x, iuv.y + h0y) - 0.5) * texelSize.xy;
	vec2 p1 = (vec2(iuv.x + h1x, iuv.y + h0y) - 0.5) * texelSize.xy;
	vec2 p2 = (vec2(iuv.x + h0x, iuv.y + h1y) - 0.5) * texelSize.xy;
	vec2 p3 = (vec2(iuv.x + h1x, iuv.y + h1y) - 0.5) * texelSize.xy;
	

  vec4 result;
  if (uvorg.x <= .5)
  {
    result = g0(fuv.y) * (g0x * texture(tex, p0)  +
                        g1x * texture(tex, p1)) +
           g1(fuv.y) * (g0x * texture(tex, p2)  +
                        g1x * texture(tex, p3));
  }
  else
  {
    result = g0(fuv.y) * mix(texture(tex, p0), texture(tex, p1), g0x) +
           g1(fuv.y) * (g0x * texture(tex, p2)  +
                        g1x * texture(tex, p3));
  }
    result = g0(fuv.y) * (g0x * texture(tex, p0)  +
                        g1x * texture(tex, p1)) +
           g1(fuv.y) * (g0x * texture(tex, p2)  +
                        g1x * texture(tex, p3));
  if (abs(uvorg.x-.5) <= .05)
  {
    result = vec4(1.);
  }
  return result;
}















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
  //rgb.rgb = abs(texture_bicubic(img, pos).rgb-rgb.rgb)*20.;
  //rgb.r = .8;
  return rgb;
}
