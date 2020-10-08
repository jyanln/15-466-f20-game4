#include "TextProgram.hpp"

#include "gl_compile_program.hpp"
#include "gl_errors.hpp"

Load< TextProgram > text_program(LoadTagEarly);

TextProgram::TextProgram() {
  //Compile vertex and fragment shaders using the convenient 'gl_compile_program' helper function:
  program = gl_compile_program(
      //vertex shader:
      "#version 330\n"
      "in vec4 Position;\n"
      "in vec4 Color;\n"
      "in vec2 TexCoord;\n"
      "out vec4 color;\n"
      "out vec2 texCoord;\n"
      "void main() {\n"
      "	gl_Position = Position;\n"
      "	color = Color;\n"
      "	texCoord = TexCoord;\n"
      "}\n"
      ,
      //fragment shader:
      "#version 330\n"
      "uniform sampler2D TEX;\n"
      "in vec4 color;\n"
      "in vec2 texCoord;\n"
      "out vec4 fragColor;\n"
      "void main() {\n"
      "	fragColor = texture(TEX, texCoord) * color;\n"
      "}\n"
      );
  //As you can see above, adjacent strings in C/C++ are concatenated.
  // this is very useful for writing long shader programs inline.

  //look up the locations of vertex attributes:
  Position_vec4 = glGetAttribLocation(program, "Position");
  Color_vec4 = glGetAttribLocation(program, "Color");
  TexCoord_vec2 = glGetAttribLocation(program, "TexCoord");

  //look up the locations of uniforms:
  GLuint TEX_sampler2D = glGetUniformLocation(program, "TEX");

  //set TEX to always refer to texture binding zero:
  glUseProgram(program); //bind program -- glUniform* calls refer to this program now

  glUniform1i(TEX_sampler2D, 0); //set TEX to sample from GL_TEXTURE0

  glUseProgram(0); //unbind program -- glUniform* calls refer to ??? now
}

TextProgram::~TextProgram() {
  glDeleteProgram(program);
  program = 0;
}

GLuint TextProgram::gen_vertex_array(GLuint vertex_buffer) const {
  GLuint vao;
  glGenVertexArrays(1, &vao);
  glBindVertexArray(vao);
  glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);

  glEnableVertexAttribArray(Position_vec4);
  glVertexAttribPointer(Position_vec4, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex),
      (const void*) offsetof(Vertex, pos));

  glEnableVertexAttribArray(Color_vec4);
  glVertexAttribPointer(Color_vec4, 4, GL_UNSIGNED_BYTE, GL_TRUE,
      sizeof(Vertex), (const void*) offsetof(Vertex, color));

  glEnableVertexAttribArray(TexCoord_vec2);
  glVertexAttribPointer(TexCoord_vec2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex),
      (const void*) offsetof(Vertex, tex));

  return vao;
}
