#pragma once

#include "GL.hpp"
#include "Load.hpp"

#include <glm/glm.hpp>

// Shader program that draws text, which are vertices based on a texture
// Written based on ColorTextureProgram and DrawLines
struct TextProgram {
	TextProgram();
	~TextProgram();

	GLuint program = 0;
	//Attribute (per-vertex variable) locations:
	GLuint Position_vec4 = -1U;
	GLuint Color_vec4 = -1U;
	GLuint TexCoord_vec2 = -1U;
	//Uniform (per-invocation variable) locations:
	GLuint OBJECT_TO_CLIP_mat4 = -1U;
	//Textures:
	//TEXTURE0 - texture that is accessed by TexCoord

  struct Vertex {
		glm::vec2 pos;
		glm::u8vec4 color;
		glm::vec2 tex;
	};

  GLuint gen_vertex_array(GLuint vertex_buffer) const;
};

extern Load< TextProgram > text_program;
