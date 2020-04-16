// Copyright 2020 The FreeAge authors
// This file is part of FreeAge, licensed under the new BSD license.
// See the COPYING file in the project root for the license text.

#pragma once

#include <memory>

#include <QOpenGLFunctions_3_2_Core>

#include "FreeAge/client/shader_program.hpp"

/// Shader for rendering sprites.
class SpriteShader {
 public:
  SpriteShader(bool shadow, bool outline);
  ~SpriteShader();
  
  inline ShaderProgram* GetProgram() { return program.get(); }
  
  void UseProgramAndSetAttribPointers(QOpenGLFunctions_3_2_Core* f);
  
  inline GLint GetTextureLocation() const { return texture_location; }
  inline GLint GetPlayerColorsTextureLocation() const { return playerColorsTexture_location; }
  inline GLint GetViewMatrixLocation() const { return viewMatrix_location; }
  inline GLint GetTextureSizeLocation() const { return textureSize_location; }
  inline GLint GetPlayerColorsTextureSizeLocation() const { return playerColorsTextureSize_location; }
  
  inline int GetVertexSize() const { return vertexSize; }
  
 private:
  std::shared_ptr<ShaderProgram> program;
  GLint texture_location;
  GLint playerColorsTexture_location;
  GLint viewMatrix_location;
  GLint size_location;
  GLint textureSize_location;
  GLint playerColorsTextureSize_location;
  GLint playerIndex_location;
  GLint tex_topleft_location;
  GLint tex_bottomright_location;
  GLint playerColor_location;
  GLint modulationColor_location;
  
  bool shadow;
  bool outline;
  int vertexSize;
};
