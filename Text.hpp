#pragma once

#include "Scene.hpp"
#include "TextProgram.hpp"

#include <ft2build.h>
#include FT_FREETYPE_H

#include <hb.h>
#include <hb-ft.h>

#include <map>

class Font;

struct GlyphData {
  FT_Int bitmap_left;
  FT_Int bitmap_top;
  unsigned int bitmap_width;
  unsigned int bitmap_rows;
  GLuint texture_id;

  GlyphData(FT_Int left, FT_Int top, unsigned int width, unsigned int rows, GLuint tex) :
    bitmap_left(left),
    bitmap_top(top),
    bitmap_width(width),
    bitmap_rows(rows),
    texture_id(tex) {}
};

class Text {
  public:
    static GLuint glbuf_;

    ~Text();

    void draw(const glm::uvec2& drawable_size, const glm::vec2& pos,
        glm::u8vec4 color, bool centered = false);

    std::string text_str;

    Font* font;

  private:
    Text();
    friend class Font;

    FT_Face face_;

    hb_buffer_t* hb_buf_;

    unsigned int num_glyphs_;
    hb_glyph_info_t* glyph_info_;
    hb_glyph_position_t* glyph_pos_;
};

class Font {
  public:
    static FT_Library ftlib;

    Font(const char* font_loc,
        FT_F26Dot6 size,
        const hb_script_t& hb_script = HB_SCRIPT_LATIN,
        const hb_language_t& hb_language = hb_language_from_string("en", -1),
        const hb_direction_t& hb_direction = HB_DIRECTION_LTR);
    ~Font();

    Text* gen_text(const char* text);

    // Cache of glyph textures
    std::map<hb_codepoint_t, GlyphData*> glyph_cache;
  private:
    // Font settings
    // TODO currently immutable
    hb_script_t hb_script_;
    hb_language_t hb_language_;
    hb_direction_t hb_direction_;

    // Font objects
    FT_Face face_;
    hb_font_t* hb_font_;
};
