/*
 *  Copyright (C) 2005-2018 Team Kodi
 *  This file is part of Kodi - https://kodi.tv
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 *  See LICENSES/README.md for more information.
 */

#include "GUITextureGL.h"

#include "ServiceBroker.h"
#include "Texture.h"
#include "rendering/gl/RenderSystemGL.h"
#include "utils/GLUtils.h"
#include "utils/Geometry.h"
#include "utils/log.h"
#include "windowing/WinSystem.h"

#include <cstddef>

CGUITexture* CGUITexture::CreateTexture(
    float posX, float posY, float width, float height, const CTextureInfo& texture)
{
  return new CGUITextureGL(posX, posY, width, height, texture);
}

CGUITextureGL::CGUITextureGL(
    float posX, float posY, float width, float height, const CTextureInfo& texture)
  : CGUITexture(posX, posY, width, height, texture)
{
  m_renderSystem = dynamic_cast<CRenderSystemGL*>(CServiceBroker::GetRenderSystem());
}

CGUITextureGL* CGUITextureGL::Clone() const
{
  return new CGUITextureGL(*this);
}

void CGUITextureGL::Begin(UTILS::Color color)
{
  CTexture* texture = m_texture.m_textures[m_currentFrame];
  texture->LoadToGPU();
  if (m_diffuse.size())
    m_diffuse.m_textures[0]->LoadToGPU();

  texture->BindToUnit(0);

  // Setup Colors
  m_col[0] = (GLubyte)GET_R(color);
  m_col[1] = (GLubyte)GET_G(color);
  m_col[2] = (GLubyte)GET_B(color);
  m_col[3] = (GLubyte)GET_A(color);

  bool hasAlpha = m_texture.m_textures[m_currentFrame]->HasAlpha() || m_col[3] < 255;

  if (m_diffuse.size())
  {
    if (m_col[0] == 255 && m_col[1] == 255 && m_col[2] == 255 && m_col[3] == 255 )
    {
      m_renderSystem->EnableShader(SM_MULTI);
    }
    else
    {
      m_renderSystem->EnableShader(SM_MULTI_BLENDCOLOR);
    }

    hasAlpha |= m_diffuse.m_textures[0]->HasAlpha();

    m_diffuse.m_textures[0]->BindToUnit(1);
  }
  else
  {
    if (m_col[0] == 255 && m_col[1] == 255 && m_col[2] == 255 && m_col[3] == 255)
    {
      m_renderSystem->EnableShader(SM_TEXTURE_NOBLEND);
    }
    else
    {
      m_renderSystem->EnableShader(SM_TEXTURE);
    }
  }

  if (hasAlpha)
  {
    glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE_MINUS_DST_ALPHA, GL_ONE);
    glEnable(GL_BLEND);
  }
  else
  {
    glDisable(GL_BLEND);
  }
  m_packedVertices.clear();
  m_idx.clear();
}
/*
void CGUITextureGL::End()
{
  if (m_packedVertices.size())
  {
    GLint posLoc  = m_renderSystem->ShaderGetPos();
    GLint tex0Loc = m_renderSystem->ShaderGetCoord0();
    GLint tex1Loc = m_renderSystem->ShaderGetCoord1();
    GLint uniColLoc = m_renderSystem->ShaderGetUniCol();

    GLuint VertexVBO;
    GLuint IndexVBO;

    glGenBuffers(1, &VertexVBO);
    glBindBuffer(GL_ARRAY_BUFFER, VertexVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(PackedVertex)*m_packedVertices.size(), &m_packedVertices[0], GL_STATIC_DRAW);

    if (uniColLoc >= 0)
    {
      glUniform4f(uniColLoc,(m_col[0] / 255.0f), (m_col[1] / 255.0f), (m_col[2] / 255.0f), (m_col[3] / 255.0f));
    }

    if (m_diffuse.size())
    {
      glVertexAttribPointer(tex1Loc, 2, GL_FLOAT, 0, sizeof(PackedVertex),
                            reinterpret_cast<const GLvoid*>(offsetof(PackedVertex, u2)));
      glEnableVertexAttribArray(tex1Loc);
    }

    glVertexAttribPointer(posLoc, 3, GL_FLOAT, 0, sizeof(PackedVertex),
                          reinterpret_cast<const GLvoid*>(offsetof(PackedVertex, x)));
    glEnableVertexAttribArray(posLoc);
    glVertexAttribPointer(tex0Loc, 2, GL_FLOAT, 0, sizeof(PackedVertex),
                          reinterpret_cast<const GLvoid*>(offsetof(PackedVertex, u1)));
    glEnableVertexAttribArray(tex0Loc);

    glGenBuffers(1, &IndexVBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IndexVBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(ushort)*m_idx.size(), m_idx.data(), GL_STATIC_DRAW);

    glDrawElements(GL_TRIANGLES, m_packedVertices.size()*6 / 4, GL_UNSIGNED_SHORT, 0);

    if (m_diffuse.size())
      glDisableVertexAttribArray(tex1Loc);

    glDisableVertexAttribArray(posLoc);
    glDisableVertexAttribArray(tex0Loc);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    glDeleteBuffers(1, &VertexVBO);
    glDeleteBuffers(1, &IndexVBO);
  }

  if (m_diffuse.size())
    glActiveTexture(GL_TEXTURE0);
  glEnable(GL_BLEND);

  m_renderSystem->DisableShader();
}*/

void CGUITextureGL::End()
{
  if (m_packedVertices.size())
  {
    GLint posLoc  = m_renderSystem->ShaderGetPos();
    GLint tex0Loc = m_renderSystem->ShaderGetCoord0();
    GLint tex1Loc = m_renderSystem->ShaderGetCoord1();
    GLint uniColLoc = m_renderSystem->ShaderGetUniCol();

    GLint sizeLoc = m_renderSystem->ShaderGetSize();
    GLint positionLoc = m_renderSystem->ShaderGetPosition();

    GLint defaultTriVertex = m_renderSystem->GetDefaultTriVertex();
    GLint defaultTriUV = m_renderSystem->GetDefaultTriUV();
    //GLint defaultTriLoc = m_renderSystem->ShaderGetTriLoc();
    /*
    CLog::LogF(LOGERROR, "m_coord[0].uv {}, {}", m_coord[0].u, m_coord[0].v);
    CLog::LogF(LOGERROR, "m_coord[1].uv {}, {}", m_coord[1].u, m_coord[1].v);
    CLog::LogF(LOGERROR, "m_coord[2].uv {}, {}", m_coord[2].u, m_coord[2].v);
    CLog::LogF(LOGERROR, "m_coord[3].uv {}, {}", m_coord[3].u, m_coord[3].v);
    */
    //GLuint VertexVBO;
    //GLuint IndexVBO;

    //glGenBuffers(1, &VertexVBO);
    glBindBuffer(GL_ARRAY_BUFFER, defaultTriVertex);
    //glBufferData(GL_ARRAY_BUFFER, sizeof(PackedVertex)*m_packedVertices.size(), &m_packedVertices[0], GL_STATIC_DRAW);

    if (uniColLoc >= 0)
    {
      glUniform4f(uniColLoc,(m_col[0] / 255.0f), (m_col[1] / 255.0f), (m_col[2] / 255.0f), (m_col[3] / 255.0f));
    }

    glUniform2f(sizeLoc,(abs(m_coord[0].u - m_coord[1].u) / (1920./2.)), (abs(m_coord[0].v - m_coord[3].v) / (1080./2.)));// * (16./9.));
    //glUniform2f(positionLoc,m_vertex.x1, m_vertex.x2);
    //glUniform2f(positionLoc,(m_coord[0].u / 1920.) - 1., (m_coord[0].v / 1080.) - 1.);
    //glUniform2f(positionLoc,(m_coord[0].u / 1920.) - 1., (m_coord[0].v / 1080.));
    //glUniform2f(sizeLoc,(1.), (1.));
    glUniform2f(positionLoc,(-1.) + (m_coord[0].u / 1920.),(1) - (m_coord[0].v / 1080.));
    //glUniform2f(positionLoc,(m_vertex.Width() / (1920.) - 1.) + m_coord[0].u / (1920.), (m_vertex.Height() / (1080.) - 1.) + ( 2. - m_coord[2].v / 1080.));
    if (m_diffuse.size())
    {
      glVertexAttribPointer(tex1Loc, 2, GL_FLOAT, 0, sizeof(PackedVertex),
                            reinterpret_cast<const GLvoid*>(offsetof(PackedVertex, u2)));
      glEnableVertexAttribArray(tex1Loc);
    }

    //glVertexAttribPointer(posLoc, 3, GL_FLOAT, 0, 0, &defaultTriVertex);
    glVertexAttribPointer(posLoc, 2, GL_FLOAT, 0, 0, 0);
    glEnableVertexAttribArray(posLoc);
    

    // UV coordinates
    //GLuint UVVBO;
    //glGenBuffers(1, &VertexVBO);

    glBindBuffer(GL_ARRAY_BUFFER, defaultTriUV);
    glVertexAttribPointer(tex0Loc, 2, GL_FLOAT, 0, 0, 0);
    //glVertexAttribPointer(tex0Loc, 2, GL_FLOAT, 0, 0, 0);
    glEnableVertexAttribArray(tex0Loc);
    
    //glGenBuffers(1, &IndexVBO);
    //glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IndexVBO);
    //glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(ushort)*m_idx.size(), m_idx.data(), GL_STATIC_DRAW);

    //glDrawElements(GL_TRIANGLES, m_packedVertices.size()*6 / 4, GL_UNSIGNED_SHORT, 0);
    //glScissor(GLint(m_coord[0].u), GLint(m_coord[0].v) - GLint(abs(m_coord[0].v - m_coord[3].v)), GLint(abs(m_coord[0].u - m_coord[1].u)), GLint(abs(m_coord[0].v - m_coord[3].v)));
    //glScissor(GLint(m_coord[0].u), GLint(m_coord[0].v), GLint(500.), GLint(500.));

    GLint x = GLint(m_coord[0].u);
    GLint y = 2160 - GLint(m_coord[2].v);
    glScissor(x, y, abs(m_coord[0].u - m_coord[1].u), abs(m_coord[0].v - m_coord[3].v));
    glEnable(GL_SCISSOR_TEST);
    glDrawArrays(GL_TRIANGLES, 0, 3);
    glDisable(GL_SCISSOR_TEST);

    if (m_diffuse.size())
      glDisableVertexAttribArray(tex1Loc);

    glDisableVertexAttribArray(posLoc);
    glDisableVertexAttribArray(tex0Loc);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    //glDeleteBuffers(1, &VertexVBO);
    //glDeleteBuffers(1, &IndexVBO);
  }

  if (m_diffuse.size())
    glActiveTexture(GL_TEXTURE0);
  glEnable(GL_BLEND);

  m_renderSystem->DisableShader();
}

void CGUITextureGL::Draw(float *x, float *y, float *z, const CRect &texture, const CRect &diffuse, int orientation)
{
  m_uvTex[0] = {texture.x1, texture.y1};
  m_uvTex[1] = {texture.x1, texture.y2 * 2.};
  m_uvTex[2] = {texture.x2 * 2., texture.y1};
  PackedVertex vertices[4];

  // Setup texture coordinates
  // TopLeft
  vertices[0].u1 = texture.x1;
  vertices[0].v1 = texture.y1;

  // TopRight
  if (orientation & 4)
  {
    vertices[1].u1 = texture.x1;
    vertices[1].v1 = texture.y2;
  }
  else
  {
    vertices[1].u1 = texture.x2;
    vertices[1].v1 = texture.y1;
  }

  // BottomRight
  vertices[2].u1 = texture.x2;
  vertices[2].v1 = texture.y2;

  // BottomLeft
  if (orientation & 4)
  {
    vertices[3].u1 = texture.x2;
    vertices[3].v1 = texture.y1;
  }
  else
  {
    vertices[3].u1 = texture.x1;
    vertices[3].v1 = texture.y2;
  }

  if (m_diffuse.size())
  {
    // TopLeft
    vertices[0].u2 = diffuse.x1;
    vertices[0].v2 = diffuse.y1;

    // TopRight
    if (m_info.orientation & 4)
    {
      vertices[1].u2 = diffuse.x1;
      vertices[1].v2 = diffuse.y2;
    }
    else
    {
      vertices[1].u2 = diffuse.x2;
      vertices[1].v2 = diffuse.y1;
    }

    // BottomRight
    vertices[2].u2 = diffuse.x2;
    vertices[2].v2 = diffuse.y2;

    // BottomLeft
    if (m_info.orientation & 4)
    {
      vertices[3].u2 = diffuse.x2;
      vertices[3].v2 = diffuse.y1;
    }
    else
    {
      vertices[3].u2 = diffuse.x1;
      vertices[3].v2 = diffuse.y2;
    }
  }

  for (int i=0; i<4; i++)
  {
    vertices[i].x = x[i];
    vertices[i].y = y[i];
    vertices[i].z = z[i];
    m_coord[i].u = x[i];
    m_coord[i].v = y[i];
    m_packedVertices.push_back(vertices[i]);
  }

  if ((m_packedVertices.size() / 4) > (m_idx.size() / 6))
  {
    size_t i = m_packedVertices.size() - 4;
    m_idx.push_back(i+0);
    m_idx.push_back(i+1);
    m_idx.push_back(i+2);
    m_idx.push_back(i+2);
    m_idx.push_back(i+3);
    m_idx.push_back(i+0);
  }
}

void CGUITexture::DrawQuad(const CRect& rect,
                           UTILS::Color color,
                           CTexture* texture,
                           const CRect* texCoords)
{
  CRenderSystemGL *renderSystem = dynamic_cast<CRenderSystemGL*>(CServiceBroker::GetRenderSystem());
  if (texture)
  {
    texture->LoadToGPU();
    texture->BindToUnit(0);
  }

  glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
  glEnable(GL_BLEND);          // Turn Blending On

  VerifyGLState();

  GLubyte col[4];
  GLubyte idx[4] = {0, 1, 3, 2};  //determines order of the vertices
  GLuint vertexVBO;
  GLuint indexVBO;

  struct PackedVertex
  {
    float x, y, z;
    float u1, v1;
  }vertex[4];

  if (texture)
    renderSystem->EnableShader(SM_TEXTURE);
  else
    renderSystem->EnableShader(SM_DEFAULT);

  GLint posLoc = renderSystem->ShaderGetPos();
  GLint tex0Loc = renderSystem->ShaderGetCoord0();
  GLint uniColLoc = renderSystem->ShaderGetUniCol();

  // Setup Colors
  col[0] = (GLubyte)GET_R(color);
  col[1] = (GLubyte)GET_G(color);
  col[2] = (GLubyte)GET_B(color);
  col[3] = (GLubyte)GET_A(color);

  glUniform4f(uniColLoc, col[0] / 255.0f, col[1] / 255.0f, col[2] / 255.0f, col[3] / 255.0f);

  // bottom left
  vertex[0].x = rect.x1;
  vertex[0].y = rect.y1;
  vertex[0].z = 0;

  // bottom right
  vertex[1].x = rect.x2;
  vertex[1].y = rect.y1;
  vertex[1].z = 0;

  // top right
  vertex[2].x = rect.x2;
  vertex[2].y = rect.y2;
  vertex[2].z = 0;

  // top left
  vertex[3].x = rect.x1;
  vertex[3].y = rect.y2;
  vertex[3].z = 0;

  if (texture)
  {
    CRect coords = texCoords ? *texCoords : CRect(0.0f, 0.0f, 1.0f, 1.0f);
    vertex[0].u1 = vertex[3].u1 = coords.x1;
    vertex[0].v1 = vertex[1].v1 = coords.y1;
    vertex[1].u1 = vertex[2].u1 = coords.x2;
    vertex[2].v1 = vertex[3].v1 = coords.y2;
  }

  glGenBuffers(1, &vertexVBO);
  glBindBuffer(GL_ARRAY_BUFFER, vertexVBO);
  glBufferData(GL_ARRAY_BUFFER, sizeof(PackedVertex)*4, &vertex[0], GL_STATIC_DRAW);

  glVertexAttribPointer(posLoc, 3, GL_FLOAT, 0, sizeof(PackedVertex),
                        reinterpret_cast<const GLvoid*>(offsetof(PackedVertex, x)));
  glEnableVertexAttribArray(posLoc);

  if (texture)
  {
    glVertexAttribPointer(tex0Loc, 2, GL_FLOAT, 0, sizeof(PackedVertex),
                          reinterpret_cast<const GLvoid*>(offsetof(PackedVertex, u1)));
    glEnableVertexAttribArray(tex0Loc);
  }

  glGenBuffers(1, &indexVBO);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexVBO);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(GLubyte)*4, idx, GL_STATIC_DRAW);

  glDrawElements(GL_TRIANGLE_STRIP, 4, GL_UNSIGNED_BYTE, 0);

  glDisableVertexAttribArray(posLoc);
  if (texture)
    glDisableVertexAttribArray(tex0Loc);

  glBindBuffer(GL_ARRAY_BUFFER, 0);
  glDeleteBuffers(1, &vertexVBO);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
  glDeleteBuffers(1, &indexVBO);

  renderSystem->DisableShader();
}

