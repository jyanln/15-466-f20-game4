#include "Mode.hpp"

#include "Scene.hpp"
#include "Sound.hpp"
#include "Text.hpp"
#include "data_path.hpp"

#include <glm/glm.hpp>

#include <string>
#include <vector>
#include <deque>

struct PlayMode : Mode {
	PlayMode();
	virtual ~PlayMode();

	//functions called by main loop:
	virtual bool handle_event(SDL_Event const &, glm::uvec2 const &window_size) override;
	virtual void update(float elapsed) override;
	virtual void draw(glm::uvec2 const &drawable_size) override;
  void gen_words();
  void init();

  // Game constants
  static constexpr FT_F26Dot6 main_word_size = 6000;
  static constexpr std::string_view main_word_ttf = "gong.ttf";
  static constexpr FT_F26Dot6 choices_size = 2000;
  static constexpr std::string_view choices_ttf = "gong.ttf";
  static constexpr FT_F26Dot6 menu_size = 3000;
  static constexpr std::string_view menu_ttf = "opensans-semibold.ttf";

  static constexpr std::string_view words_filepath = "google-10000-english-usa-no-swears-long.txt";

  static constexpr int choice_num = 4;

  static constexpr float max_time = 20.0f;
  static constexpr float time_decay = 1.0f;
  static constexpr float min_time = 3.0f;

  static constexpr glm::u8vec4 background_color =
    glm::u8vec4(0xE7, 0xE5, 0xE5, 0xFF);
  static constexpr glm::u8vec4 main_word_green_color =
    glm::u8vec4(0x7B, 0xE0, 0xAD, 0xFF);
  static constexpr glm::u8vec4 main_word_red_color =
    glm::u8vec4(0xEA, 0x9A, 0xA2, 0xFF);
  static constexpr glm::u8vec4 choices_color =
    glm::u8vec4(0x06, 0x60, 0x5A, 0xFF);
  static constexpr glm::u8vec4 choices_selected_color =
    glm::u8vec4(0x03, 0x31, 0x2E, 0xFF);
  static constexpr glm::u8vec4 menu_color =
    glm::u8vec4(0x40, 0x40, 0x40, 0xFF);

  static constexpr float score_mult = 100.0f;

	//----- game state -----
  Font* main_word_font;
  Text* main_word_text;

  Font* choices_font;
  Text* choices_text[choice_num]; 
  Font* menu_font;
  Text* score_text;
  Text* timer_text;

  int cursor;

  int score;
  float cur_max_time;
  float timer;

  std::vector<std::string> words;

  bool running;
};
