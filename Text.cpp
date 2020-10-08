#include "Text.hpp"

#include "Load.hpp"

#include <stdexcept>
#include <iostream>

// Static variables and initializers
FT_Library Font::ftlib;
GLuint Text::glbuf_;
unsigned int glbuf_data[] {0, 1, 2, 1, 2, 3};

Load<void> load_ftlib(LoadTagEarly, []() {
    FT_Init_FreeType(&Font::ftlib);
    });

Load<void> load_index_buffer(LoadTagEarly, [](){
    glGenBuffers(1, &Text::glbuf_);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, Text::glbuf_);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, 6 * sizeof(unsigned int),
        glbuf_data, GL_STATIC_DRAW);
    });

Text::Text() {}

Text::~Text() {
  if(hb_buf_) {
    hb_buffer_destroy(hb_buf_);
    hb_buf_ = nullptr;
  }
}

void Text::draw(const glm::uvec2& drawable_size, const glm::vec2& pos,
    glm::u8vec4 color, bool centered) {
  glUseProgram(text_program->program);

  glm::vec2 cur_pos;

  if(centered) {
    // If centered, calculate width
    //TODO can cache this calculation
    float total_width = 0.0f;
    for(unsigned int i = 0; i < num_glyphs_; i++) {
      total_width += glyph_pos_[i].x_advance / 64.0f;
    }
    cur_pos = glm::vec2((pos.x + 1.0f) * drawable_size.x / 2.0f - total_width / 2,
        (pos.y + 1.0f) * drawable_size.y / 2.0f);
  } else {
    cur_pos = glm::vec2((pos.x + 1.0f) * drawable_size.x / 2.0f,
        (pos.y + 1.0f) * drawable_size.y / 2.0f);
  }

  for(unsigned int i = 0; i < num_glyphs_; i++) {
    // Retrieve stored glyph data
    hb_codepoint_t code = glyph_info_[i].codepoint;
    GlyphData* data = font->glyph_cache[code];

    glm::vec2 advance = glm::vec2(glyph_pos_[i].x_advance,
        glyph_pos_[i].y_advance) / 64.0f;

    float x_ratio = 2.0f / drawable_size.x;
    float y_ratio = 2.0f / drawable_size.y;

    float start_x = (cur_pos.x + data->bitmap_left) * x_ratio - 1;
    float end_x = data->bitmap_width * x_ratio + start_x; 

    float start_y = (cur_pos.y - data->bitmap_rows + data->bitmap_top) *
      y_ratio - 1;
    float end_y = data->bitmap_rows * y_ratio + start_y; 

    TextProgram::Vertex vertices[] {
      {{start_x, start_y}, color, {0, 1}},
        {{end_x, start_y}, color, {1, 1}},
        {{start_x, end_y}, color, {0, 0}},
        {{end_x, end_y}, color, {1, 0}}
    };

    GLuint vertex_buffer, vertex_array;

    glGenBuffers(1, &vertex_buffer);
    vertex_array = text_program->gen_vertex_array(vertex_buffer);

    glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);
    glBufferData(GL_ARRAY_BUFFER, 4 * sizeof(TextProgram::Vertex), static_cast<const void*>(vertices), GL_STATIC_DRAW);

    glBindVertexArray(vertex_array);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, glbuf_);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, data->texture_id);

    // Render glyphs
    // Partially referenced from:
    // https://github.com/tangrams/harfbuzz-example/blob/master/src/hbshaper.h
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, static_cast<const void*>(0));

    glDeleteBuffers(1, &vertex_buffer);
    glDeleteVertexArrays(1, &vertex_array);

    cur_pos += advance;
  }
}

Font::Font(const char* hb_font_loc,
    FT_F26Dot6 size,
    const hb_script_t& hb_script,
    const hb_language_t& hb_language,
    const hb_direction_t& hb_direction) :
  hb_script_(hb_script),
  hb_language_(hb_language),
  hb_direction_(hb_direction) {
    FT_Error err = FT_New_Face(Font::ftlib, hb_font_loc, 0, &face_);
    if(err) {
      std::cerr << "Error loading font at " << hb_font_loc << "with error code"
        << err << std::endl;
      //TODO
      exit(-2);
    }

    FT_Set_Char_Size(face_, 0, size, 0, 0);
    hb_font_ = hb_ft_font_create(face_, nullptr);
  }

Font::~Font() {
  if(hb_font_) {
    hb_font_destroy(hb_font_);
    hb_font_ = nullptr;
  }
}

Text* Font::gen_text(const char* text_str) {
  Text* text = new Text();
  text->font = this;
  text->face_ = face_;

  text->text_str = text_str;

  // Setup hb_buffer for text
  // Referenced from https://harfbuzz.github.io/ch03s03.html
  text->hb_buf_ = hb_buffer_create();
  hb_buffer_add_utf8(text->hb_buf_, text_str, -1, 0, -1);
  hb_buffer_set_direction(text->hb_buf_, hb_direction_);
  hb_buffer_set_script(text->hb_buf_, hb_script_);
  hb_buffer_set_language(text->hb_buf_, hb_language_);
  hb_shape(hb_font_, text->hb_buf_, nullptr, 0);
  text->glyph_info_ = hb_buffer_get_glyph_infos(text->hb_buf_, &text->num_glyphs_);
  text->glyph_pos_ = hb_buffer_get_glyph_positions(text->hb_buf_, &text->num_glyphs_);

  for(unsigned int i = 0; i < text->num_glyphs_; i++) {
    hb_codepoint_t code = text->glyph_info_[i].codepoint;
    // Check if cached
    if(glyph_cache.count(code)) continue;

    // Render glyphs
    // Partially referenced from:
    // https://github.com/tangrams/harfbuzz-example/blob/master/src/hbshaper.h
    FT_Load_Glyph(face_, code, FT_LOAD_DEFAULT);

    FT_GlyphSlot slot = face_->glyph;
    FT_Render_Glyph(slot, FT_RENDER_MODE_NORMAL);

    FT_Bitmap& bitmap = slot->bitmap;

    GlyphData* data = new GlyphData(
        slot->bitmap_left,
        slot->bitmap_top,
        bitmap.width,
        bitmap.rows,
        0);

    glyph_cache[code] = data;

    GLuint &texture_id = data->texture_id;

    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glGenTextures(1, &texture_id);
    glBindTexture(GL_TEXTURE_2D, texture_id);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_R, GL_ONE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_G, GL_ONE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_B, GL_ONE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_A, GL_RED);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, bitmap.width, bitmap.rows, 0, GL_RED, GL_UNSIGNED_BYTE, bitmap.buffer);

    glBindTexture(GL_TEXTURE_2D, 0);
  }

  return text;
}
