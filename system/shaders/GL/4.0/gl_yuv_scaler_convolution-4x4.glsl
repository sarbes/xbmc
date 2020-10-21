vec4[4] _scaler_load4x4_0(sampler2D sampler, vec2 pos)
{
  vec4[4] tex4x4;
  vec4 tex2x2 = textureGather(sampler, pos, 0);
  tex4x4[0].xy = tex2x2.wz;
  tex4x4[1].xy = tex2x2.xy;
  tex2x2 = textureGatherOffset(sampler, pos, ivec2(2,0), 0);
  tex4x4[0].zw = tex2x2.wz;
  tex4x4[1].zw = tex2x2.xy;
  tex2x2 = textureGatherOffset(sampler, pos, ivec2(0,2), 0);
  tex4x4[2].xy = tex2x2.wz;
  tex4x4[3].xy = tex2x2.xy;
  tex2x2 = textureGatherOffset(sampler, pos, ivec2(2,2), 0);
  tex4x4[2].zw = tex2x2.wz;
  tex4x4[3].zw = tex2x2.xy;
  return tex4x4;
}

float _scaler_filter_0(sampler2D sampler, vec2 coord)
{
  vec2 pos = coord + m_step * 0.5;
  vec2 f = fract(pos / m_step);

  vec4 linetaps = texture(m_kernelTex, 1.0 - f.x);
  vec4 coltaps = texture(m_kernelTex, 1.0 - f.y);
  linetaps /= linetaps.r + linetaps.g + linetaps.b + linetaps.a;
  coltaps /= coltaps.r + coltaps.g + coltaps.b + coltaps.a;
  mat4 conv;
  conv[0] = linetaps * coltaps.x;
  conv[1] = linetaps * coltaps.y;
  conv[2] = linetaps * coltaps.z;
  conv[3] = linetaps * coltaps.w;

  vec2 startPos = (-1.0 - f) * m_step + pos;
  vec4[4] tex4x4 = _scaler_load4x4_0(sampler, startPos);
  vec4 imageLine0 = tex4x4[0];
  vec4 imageLine1 = tex4x4[1];
  vec4 imageLine2 = tex4x4[2];
  vec4 imageLine3 = tex4x4[3];

  return dot(imageLine0, conv[0]) +
         dot(imageLine1, conv[1]) +
         dot(imageLine2, conv[2]) +
         dot(imageLine3, conv[3]);
}

vec4 scaler()
{
  vec4 yuv;
#if defined(XBMC_YV12)

  yuv = vec4(_scaler_filter_0(m_sampY, stretch(m_cordY)),
             texture(m_sampU, stretch(m_cordU)).r,
             texture(m_sampV, stretch(m_cordV)).r,
             1.0);

#elif defined(XBMC_NV12)

  yuv = vec4(_scaler_filter_0(m_sampY, stretch(m_cordY)),
             texture(m_sampU, stretch(m_cordU)).rg,
             1.0);

#endif

  return yuv;
}
