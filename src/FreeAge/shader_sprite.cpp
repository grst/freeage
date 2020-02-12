#include "FreeAge/shader_sprite.h"

#include "FreeAge/logging.h"

SpriteShader::SpriteShader(bool shadow) {
  program.reset(new ShaderProgram());
  
  CHECK(program->AttachShader(
      "#version 330 core\n"
      "in vec3 in_position;\n"
      "uniform mat2 u_viewMatrix;\n"
      "void main() {\n"
      "  gl_Position = vec4(u_viewMatrix[0][0] * in_position.x + u_viewMatrix[1][0], u_viewMatrix[0][1] * in_position.y + u_viewMatrix[1][1], in_position.z, 1);\n"
      "}\n",
      ShaderProgram::ShaderType::kVertexShader));
  
  CHECK(program->AttachShader(
      "#version 330 core\n"
      "#extension GL_EXT_geometry_shader : enable\n"
      "layout(points) in;\n"
      "layout(triangle_strip, max_vertices = 4) out;\n"
      "\n"
      "uniform vec2 u_size;\n"
      "uniform vec2 u_tex_topleft;\n"
      "uniform vec2 u_tex_bottomright;\n"
      "\n"
      "out vec2 texcoord;\n"
      "\n"
      "void main() {\n"
      "  gl_Position = vec4(gl_in[0].gl_Position.x, gl_in[0].gl_Position.y, gl_in[0].gl_Position.z, 1.0);\n"
      "  texcoord = vec2(u_tex_topleft.x, u_tex_topleft.y);\n"
      "  EmitVertex();\n"
      "  gl_Position = vec4(gl_in[0].gl_Position.x + u_size.x, gl_in[0].gl_Position.y, gl_in[0].gl_Position.z, 1.0);\n"
      "  texcoord = vec2(u_tex_bottomright.x, u_tex_topleft.y);\n"
      "  EmitVertex();\n"
      "  gl_Position = vec4(gl_in[0].gl_Position.x, gl_in[0].gl_Position.y - u_size.y, gl_in[0].gl_Position.z, 1.0);\n"
      "  texcoord = vec2(u_tex_topleft.x, u_tex_bottomright.y);\n"
      "  EmitVertex();\n"
      "  gl_Position = vec4(gl_in[0].gl_Position.x + u_size.x, gl_in[0].gl_Position.y - u_size.y, gl_in[0].gl_Position.z, 1.0);\n"
      "  texcoord = vec2(u_tex_bottomright.x, u_tex_bottomright.y);\n"
      "  EmitVertex();\n"
      "  \n"
      "  EndPrimitive();\n"
      "}\n",
      ShaderProgram::ShaderType::kGeometryShader));
  
  if (shadow) {
    CHECK(program->AttachShader(
        "#version 330 core\n"
        "layout(location = 0) out vec4 out_color;\n"
        "\n"
        "in vec2 texcoord;\n"
        "\n"
        "uniform sampler2D u_texture;\n"
        "\n"
        "void main() {\n"
        "  out_color = vec4(0, 0, 0, 1.5 * texture(u_texture, texcoord.xy).r);\n"  // TODO: Magic factor 1.5 makes it look nicer (darker shadows)
        "}\n",
        ShaderProgram::ShaderType::kFragmentShader));
  } else {
    CHECK(program->AttachShader(
        "#version 330 core\n"
        "layout(location = 0) out vec4 out_color;\n"
        "\n"
        "in vec2 texcoord;\n"
        "\n"
        "uniform sampler2D u_texture;\n"
        "\n"
        "void main() {\n"
        "  out_color = texture(u_texture, texcoord.xy);\n"
        "  if (out_color.a < 0.5) {\n"
        "    discard;\n"
        "  }\n"
        "  out_color.a = 1;\n"  // TODO: Instead of setting a to 1 here, disable blending?
        "}\n",
        ShaderProgram::ShaderType::kFragmentShader));
  }
  
  CHECK(program->LinkProgram());
  
  program->UseProgram();
  
  texture_location =
      program->GetUniformLocationOrAbort("u_texture");
  viewMatrix_location =
      program->GetUniformLocationOrAbort("u_viewMatrix");
  size_location =
      program->GetUniformLocationOrAbort("u_size");
  tex_topleft_location =
      program->GetUniformLocationOrAbort("u_tex_topleft");
  tex_bottomright_location =
      program->GetUniformLocationOrAbort("u_tex_bottomright");
}

SpriteShader::~SpriteShader() {
  program.reset();
}
