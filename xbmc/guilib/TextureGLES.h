/*
 *  Copyright (C) 2024 Team Kodi
 *  This file is part of Kodi - https://kodi.tv
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 *  See LICENSES/README.md for more information.
 */

#pragma once

#include "Texture.h"
#include "guilib/TextureGLESFormatMap.h"

#include "system_gl.h"

using namespace KODI::GUILIB::GLES;

class CGLESTexture : public CTexture
{
public:
  CGLESTexture(unsigned int width = 0, unsigned int height = 0, XB_FMT format = XB_FMT_A8R8G8B8);
  ~CGLESTexture() override;

  void CreateTextureObject() override;
  void DestroyTextureObject() override;
  void LoadToGPU() override;
  void BindToUnit(unsigned int unit) override;

protected:
  void SetSwizzle(bool swapRB);
  void SwapBlueRedSwizzle(GLint& component);
  TextureFormatGLES GetFormatGLES20(KD_TEX_FMT textureFormat);
  TextureFormatGLES GetFormatGLES30(KD_TEX_FMT textureFormat);

  GLuint m_texture = 0;
  bool m_isGLESVersion30orNewer{false};
};
