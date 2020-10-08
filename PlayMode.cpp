#include "PlayMode.hpp"

#include "Mesh.hpp"
#include "Load.hpp"
#include "gl_errors.hpp"
#include "data_path.hpp"

#include <glm/gtc/type_ptr.hpp>

#include <fstream>
#include <random>
#include <set>

/** TODO sound
Load< Sound::Sample > dusty_floor_sample(LoadTagDefault, []() -> Sound::Sample const * {
	return new Sound::Sample(data_path("dusty-floor.opus"));
});
*/

PlayMode::PlayMode() {
  // Load fonts
  main_word_font = new Font(data_path(std::string(main_word_ttf)).c_str(),
      main_word_size);
  choices_font = new Font(data_path(std::string(choices_ttf)).c_str(),
      choices_size);
  menu_font = new Font(data_path(std::string(menu_ttf)).c_str(),
      menu_size);

  // hack: empty texts before first wordgen
  main_word_text = menu_font->gen_text("");
  for(int i = 0; i < choice_num; i++) {
    choices_text[i] = menu_font->gen_text("");
  }
  score_text = menu_font->gen_text("");
  timer_text = menu_font->gen_text("");

  // Load words
  std::string words_datapath = data_path(std::string(words_filepath));
  std::ifstream words_file(words_datapath.c_str());

  std::string str;
  while(std::getline(words_file, str)) words.push_back(str);

  words_file.close();
  
  init();
}

PlayMode::~PlayMode() {
}

void PlayMode::gen_words() {
  // Get 5 random words
  std::vector<std::string> wordset;
  std::sample(words.begin(), words.end(), std::back_inserter(wordset), choice_num + 1, std::mt19937{std::random_device{}()});

  delete main_word_text;
  main_word_text = main_word_font->gen_text(wordset[0].c_str());

  for(int i = 0; i < choice_num; i++) {
    delete choices_text[i];
    choices_text[i] = choices_font->gen_text(wordset[i + 1].c_str());
  }
}

void PlayMode::init() {
 cur_max_time = max_time; 
 cursor = 0;
 score = 0;
 timer = cur_max_time;
 gen_words();
 running = true;
}

void confirm_choice(int selection) {
}

bool PlayMode::handle_event(SDL_Event const &evt, glm::uvec2 const &window_size) {

	if (evt.type == SDL_KEYDOWN) {
    if(!running) {
      init();
      return true;
    }

		if (evt.key.keysym.sym == SDLK_LEFT) {
			cursor--;
      if(cursor < 0) cursor = choice_num - 1;

			return true;
		} else if (evt.key.keysym.sym == SDLK_RIGHT) {
      cursor++;
      if(cursor >= choice_num) cursor = 0;

			return true;
		} else if (evt.key.keysym.sym == SDLK_SPACE) {
      // Calculate score
      std::string word1 = main_word_text->text_str;
      std::string word2 = choices_text[cursor]->text_str;

      std::set<char> chars1;
      for(auto it = word1.cbegin(); it != word1.cend(); it++) {
        chars1.insert(*it);
      }
      std::set<char> chars2;
      for(auto it = word2.cbegin(); it != word2.cend(); it++) {
        chars2.insert(*it);
      }
      std::set<char> common;

      std::set_intersection(chars1.begin(), chars1.end(),
          chars2.begin(), chars2.end(),
          std::inserter(common, common.begin()));

      int raw_score = chars2.size() - common.size();

      score += raw_score * score_mult * timer / cur_max_time;

      // Decay max time if applicable
      if(cur_max_time > min_time) cur_max_time -= time_decay;

      timer = cur_max_time;

      gen_words();
      cursor = 0;

			return true;
		}
	}

	return false;
}

void PlayMode::update(float elapsed) {
  if(!running) return;

  timer -= elapsed;
  if(timer < 0) {
    running = false;
  }

  delete score_text;
  score_text = menu_font->gen_text(std::to_string(score).c_str());
  delete timer_text;
  timer_text = menu_font->gen_text(std::to_string(timer).c_str());
}

void PlayMode::draw(glm::uvec2 const &drawable_size) {
  glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

  glClearColor(0.5f, 0.5f, 0.5f, 1.0f);

  glClearDepth(1.0);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  glEnable(GL_DEPTH_TEST);
  glDepthFunc(GL_LESS);
  glDisable(GL_DEPTH_TEST);

  glm::u8vec4 color = timer < (0.25f * cur_max_time) ? main_word_red_color :
    main_word_green_color;
  main_word_text->draw(drawable_size, glm::vec2(0.0f, 0.2f), color, true);

  int top_num = choice_num / 2 + choice_num % 2;
  int bot_num = choice_num / 2;
  float top_pos_offset = 2.0f / (top_num + 1);
  float top_pos_start = -1.0f + top_pos_offset;
  for(int i = 0; i < top_num; i++) {
    color = i == cursor ? choices_selected_color : choices_color;
    choices_text[i]->draw(drawable_size,
        glm::vec2(top_pos_start + top_pos_offset * i, -0.2f), color, true);
  }
  float bot_pos_offset = 2.0f / (bot_num + 1);
  float bot_pos_start = -1.0f + bot_pos_offset;
  for(int i = 0; i < bot_num; i++) {
    int index = top_num + i;
    color = index == cursor ? choices_selected_color : choices_color;
    choices_text[index]->draw(drawable_size,
        glm::vec2(bot_pos_start + bot_pos_offset * i, -0.4f), color, true);
  }

  score_text->draw(drawable_size, glm::vec2(0.8f, 0.8f), menu_color);
  timer_text->draw(drawable_size, glm::vec2(0.8f, 0.6f), menu_color);
}
