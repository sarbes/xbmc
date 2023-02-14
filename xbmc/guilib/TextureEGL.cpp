/*
 *  Copyright (C) 2005-2018 Team Kodi
 *  This file is part of Kodi - https://kodi.tv
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 *  See LICENSES/README.md for more information.
 */

#include "TextureEGL.h"

#include "ServiceBroker.h"
#include "guilib/TextureGL.h"
#include "guilib/TextureManager.h"
#include "settings/AdvancedSettings.h"
#include "settings/SettingsComponent.h"
#include "utils/BufferObject.h"
#include "utils/EGLImage.h"
#include "utils/log.h"
#include "windowing/GraphicContext.h"
#include "windowing/linux/WinSystemEGL.h"
#include "windowing/WinSystem.h"
#ifdef HAVE_X11
#include "windowing/X11/WinSystemX11.h"
#ifndef HAS_GLES
#include "windowing/X11/WinSystemX11GLContext.h"
#else
#include "windowing/X11/WinSystemX11GLESContext.h"
#endif
#endif

#include <memory>
#include <GL/gl.h>



CEGLTexture::CEGLTexture(uint32_t width, uint32_t height, uint32_t format)
  : CGLTexture(width, height, format)
{
}

CEGLTexture::~CEGLTexture()
{
  DestroyTextureObject();
}

void CEGLTexture::DestroyTextureObject()
{
  if (m_eglImage)
  {
    m_eglImage->DestroyImage();
    m_eglImage = nullptr;
  }
  if (m_bufferObject)
  {
    m_pixels = nullptr;
    m_bufferObject->ReleaseMemory();
    m_bufferObject->DestroyBufferObject();
  }
  CGLTexture::DestroyTextureObject();
}

void CEGLTexture::LoadToGPU()
{
  if (!m_isDMATexture)
    return CGLTexture::LoadToGPU();

  if (!m_eglImage || m_uploadedToGPU)
    return;

  if (!m_texture)
    glGenTextures(1, (GLuint*) &m_texture);

  GLint textureTarget = GL_TEXTURE_2D;

  glBindTexture(textureTarget, m_texture);

  glTexParameteri(textureTarget, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(textureTarget, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(textureTarget, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(textureTarget, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

  m_eglImage->UploadImage(textureTarget);

  glBindTexture(textureTarget, 0);
  m_uploadedToGPU = true;
}

void CEGLTexture::Allocate(uint32_t format)
{
  if (m_textureUploadMethod == TEXTURE_UPLOAD_METHOD::TEXTURE_UPLOAD_NORMAL ||
      (m_textureUploadMethod == TEXTURE_UPLOAD_METHOD::TEXTURE_UPLOAD_PREFER_DMA && 
       !CServiceBroker::GetSettingsComponent()->GetAdvancedSettings()->m_guiDMATextureUpload)
  )
    return CGLTexture::Allocate(format);

  m_isDMATexture = true;

  uint32_t fourccFormat = 0;
  switch (format)
  {
    case XB_FMT_A8R8G8B8:
      fourccFormat = DRM_FORMAT_ARGB8888;
      break;
    default:
      CLog::Log(LOGERROR, "CEGLTexture::{} No suitable format found", __FUNCTION__);
      return;
  }

  m_bufferObject = CBufferObject::GetBufferObject(false);

  // Some GPUs need texture sizes with a multiple of 16
  uint32_t textureWidth = ((m_textureWidth + 15) / 16) * 16;
  uint32_t textureHeight = ((m_textureHeight + 15) / 16) * 16;
  if (!m_bufferObject || !m_bufferObject->CreateBufferObject(fourccFormat, textureWidth, textureHeight))
  {
    CLog::Log(LOGERROR, "CEGLTexture::{} Error creating buffer object", __FUNCTION__);
    return;
  }

  EGLDisplay eglDisplay = nullptr;
  // Messy. Is there a better way?
#ifndef HAVE_X11
  auto winSystemLinuxEGL = dynamic_cast<KODI::WINDOWING::LINUX::CWinSystemEGL*>(CServiceBroker::GetWinSystem());
  eglDisplay = winSystemLinuxEGL->GetEGLDisplay();

#else
#ifndef HAS_GLES
  if (!eglDisplay)
  {
  auto winSystemX11EGL = dynamic_cast<KODI::WINDOWING::X11::CWinSystemX11GLContext*>(CServiceBroker::GetWinSystem());
  eglDisplay = winSystemX11EGL->GetEGLDisplay();
  }
#endif
#ifdef HAS_GLES
  if (!eglDisplay)
  {
  auto winSystemX11EGL = dynamic_cast<KODI::WINDOWING::X11::CWinSystemX11GLESContext*>(CServiceBroker::GetWinSystem());
  eglDisplay = winSystemX11EGL->GetEGLDisplay();
  }
#endif
#endif

  if (!eglDisplay)
  {
    CLog::Log(LOGERROR, "CEGLTexture::{} Failed to query EGL Display", __FUNCTION__);
    return;
  }

  m_eglImage = std::make_unique<CEGLImage>(eglDisplay);

  CEGLImage::EglPlane plane;
  plane.fd = m_bufferObject->GetFd();
  plane.offset = 0;
  plane.pitch = GetPitch();
  plane.modifier = m_bufferObject->GetModifier();

  CEGLImage::EglAttrs attribs;
  attribs.width = m_textureWidth;
  attribs.height = m_textureHeight;
  attribs.format = fourccFormat;
  attribs.planes[0] = plane;

  if (m_eglImage->CreateImage(attribs))
  {
    m_pixels = m_bufferObject->GetMemory();
  }
  return;
}

void CEGLTexture::AfterWritingTexture()
{
  if (!m_isDMATexture)
    return;

  if (m_bufferObject)
  {
    m_bufferObject->ReleaseMemory();
    m_pixels = nullptr;
    m_bufferObject->DestroyBufferObject();
  }
}
