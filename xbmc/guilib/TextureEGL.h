/*
 *  Copyright (C) 2005-2018 Team Kodi
 *  This file is part of Kodi - https://kodi.tv
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 *  See LICENSES/README.md for more information.
 */

#pragma once

#include "TextureGL.h"
#include "guilib/Texture.h"
#include "utils/EGLImage.h"
#include "utils/IBufferObject.h"

#include "system_gl.h"

/************************************************************************/
/*    CEGLTexture                                                       */
/************************************************************************/
class CEGLTexture : public CGLTexture
{
public:
  CEGLTexture(uint32_t width = 0, uint32_t height = 0, uint32_t format = XB_FMT_A8R8G8B8);
  ~CEGLTexture() override;

  void DestroyTextureObject() override;
  void LoadToGPU() override;
  void Allocate(uint32_t format) override;

protected:
  bool SupportsDMAUpload() override { return true; }
  void AfterWritingTexture() override;
  
  std::unique_ptr<CEGLImage> m_eglImage;
  std::unique_ptr<IBufferObject> m_bufferObject;
  bool m_isDMATexture{false};
  bool m_uploadedToGPU{false};
};

