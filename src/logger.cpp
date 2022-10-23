#include "globals.h"
#include "logger.h"
#include "graphics.h"
#include "input.h"
#include <SFML/Window/Clipboard.hpp>
#include <SFML/Window/Keyboard.hpp>
#include <stdarg.h>
#include <iostream>

#define LOGGER_LINES_LIMIT 10000
#define LOGGER_LINES_ERASE_COUNT (LOGGER_LINES_LIMIT / 20 + 1)

#define KEY_AUTOREPEAT_START 0.25f
#define KEY_AUTOREPEAT_PERIOD 0.03f

#define FONT_HEIGHT 18
#define FONT_WIDTH 9
#define SAFE_AREA 8
#define LOG_LINES_PER_SCREEN (((screen_height - SAFE_AREA * 2) / FONT_HEIGHT) - 3)
#define LOG_SYMBOLS_PER_SCREEN ((screen_width - SAFE_AREA * 2) / FONT_WIDTH)



using namespace std;

DasboxLogger dasbox_logger;
static bool error_as_note = false;
static bool saved_relative_mode = false;
static bool saved_cursor_hidden = false;

std::string replace_all(std::string str, const std::string & from, const std::string & to)
{
  size_t startPos = 0;
  while((startPos = str.find(from, startPos)) != std::string::npos)
  {
    str.replace(startPos, from.length(), to);
    startPos += to.length();
  }
  return str;
}


bool ends_with_newline(const char * str)
{
  size_t len = strlen(str);
  return (len && str[len - 1] == '\n');
}

void reinterpret_error_as_note(bool is_note)
{
  error_as_note = is_note;
}

void print_error(const char * format, ...)
{
  static char buf[16000];
  va_list args;
  va_start(args, format);
  vsnprintf(buf, sizeof(buf), format, args);
  buf[sizeof(buf) - 1] = 0;
  va_end(args);

  if (error_as_note)
  {
    dasbox_logger.setState(LOGGER_NOTE);
    dasbox_logger << "Note: ";
    dasbox_logger << buf;
    if (!ends_with_newline(buf))
      dasbox_logger << "\n";
    dasbox_logger.setState(LOGGER_NORMAL);
  }
  else
  {
    dasbox_logger.setTopErrorLine();
    dasbox_logger.setState(LOGGER_ERROR);
    dasbox_logger << "\nERROR: ";
    dasbox_logger << buf;
    dasbox_logger << "\n";
    dasbox_logger.setState(LOGGER_NORMAL);
  }

  if (exit_on_error)
    exit(exit_code_on_error);
}

void print_exception(const char * format, ...)
{
  static char buf[16000];
  va_list args;
  va_start(args, format);
  vsnprintf(buf, sizeof(buf), format, args);
  buf[sizeof(buf) - 1] = 0;
  va_end(args);
  dasbox_logger.setTopErrorLine();
  dasbox_logger.setState(LOGGER_ERROR);
  dasbox_logger << "\nEXCEPTION: ";
  dasbox_logger << buf;
  dasbox_logger << "\n";
  dasbox_logger.setState(LOGGER_NORMAL);
}

void print_note(const char * format, ...)
{
  static char buf[16000];
  va_list args;
  va_start(args, format);
  vsnprintf(buf, sizeof(buf), format, args);
  buf[sizeof(buf) - 1] = 0;
  va_end(args);
  dasbox_logger.setState(LOGGER_NOTE);
  dasbox_logger << "Note: ";
  dasbox_logger << buf;
  if (!ends_with_newline(buf))
    dasbox_logger << "\n";
  dasbox_logger.setState(LOGGER_NORMAL);
}

void print_warning(const char * format, ...)
{
  static char buf[16000];
  va_list args;
  va_start(args, format);
  vsnprintf(buf, sizeof(buf), format, args);
  buf[sizeof(buf) - 1] = 0;
  va_end(args);
  dasbox_logger.setState(LOGGER_WARNING);
  dasbox_logger << "WARNING: ";
  dasbox_logger << buf;
  if (!ends_with_newline(buf))
    dasbox_logger << "\n";
  dasbox_logger.setState(LOGGER_NORMAL);
}

void print_text(const char * format, ...)
{
  static char buf[16000];
  va_list args;
  va_start(args, format);
  vsnprintf(buf, sizeof(buf), format, args);
  buf[sizeof(buf) - 1] = 0;
  va_end(args);
  dasbox_logger.setState(LOGGER_NORMAL);
  dasbox_logger << buf;
  if (!ends_with_newline(buf))
    dasbox_logger << "\n";
  dasbox_logger.setState(LOGGER_NORMAL);
}


void DasboxLogger::clear()
{
  logStrings.clear();
  lineColors.clear();
  topLine = 0;
  topErrorLine = -1;
  curColor = NORMAL_LINE_COLOR;
  buf.clear();
}

void DasboxLogger::setTopErrorLine()
{
  if (topErrorLine < 0)
    topErrorLine = int(dasbox_logger.logStrings.size());
}

void DasboxLogger::output()
{
  int newPos = tellp();
  if (newPos != pos)
  {
    addString(string(data.data() + pos, newPos - pos));
    pos = newPos;
  }
}

void DasboxLogger::printAllLog()
{
  for (auto & s : logStrings)
    cout << s;
}

uint32_t DasboxLogger::setLogColor(uint32_t color)
{
  uint32_t c = curColor;
  curColor = color;
  return c;
}

void DasboxLogger::applyStateColor()
{
  switch (state)
  {
  case LOGGER_ERROR: curColor = ERROR_LINE_COLOR; break;
  case LOGGER_WARNING: curColor = WARNING_LINE_COLOR; break;
  case LOGGER_NOTE: curColor = NOTE_LINE_COLOR; break;
  default: curColor = NORMAL_LINE_COLOR; break;
  }
}


void DasboxLogger::addString(const string & s)
{
#ifndef NDEBUG
  bool duplicateLog = true;
#else
  bool duplicateLog = log_to_console;
#endif
  if (duplicateLog)
  {
    if (state == LOGGER_ERROR)
    {
      cerr << s;
      cerr.flush();
    }
    else
    {
      cout << s;
      cout.flush();
    }
  }

  if (logStrings.empty())
  {
    logStrings.push_back(string(""));
    lineColors.push_back(curColor);
  }

  if (logStrings.back().empty())
    lineColors.back() = curColor;

  logStrings.back().append(replace_all(s, string("\t"), string("    ")));

  for (;;)
  {
    size_t pos = logStrings.back().find('\n');

    if (pos != string::npos)
      pos++;

    if (pos != string::npos && pos >= LOG_SYMBOLS_PER_SCREEN)
      pos = LOG_SYMBOLS_PER_SCREEN;

    if (pos == string::npos && logStrings.back().length() >= LOG_SYMBOLS_PER_SCREEN)
      pos = LOG_SYMBOLS_PER_SCREEN;

    if (pos == string::npos)
      break;

    string newString = string(logStrings.back().c_str() + pos);
    logStrings.back().resize(pos);
    logStrings.push_back(newString);
    lineColors.push_back(curColor);
  }

  if (logStrings.size() > LOGGER_LINES_LIMIT)
  {
    logStrings.erase(logStrings.begin(), logStrings.begin() + LOGGER_LINES_ERASE_COUNT);
    lineColors.erase(lineColors.begin(), lineColors.begin() + LOGGER_LINES_ERASE_COUNT);
    if (topErrorLine >= 0)
    {
      topErrorLine -= LOGGER_LINES_ERASE_COUNT;
      if (topErrorLine < 0)
        topErrorLine = 0;
    }
    topLine -= LOGGER_LINES_ERASE_COUNT;
    if (topLine < 0)
      topLine = 0;
  }
}

static float scroll_accum = 0.0f;
static float time_to_reset_scroll_accum = 2.0f;

static int select_from_line = -1;
static int select_to_line = -1;
static int select_from_pos = -1;
static int select_to_pos = -1;

static bool mouse_down = false;
static das::int2 select_mouse_down = das::int2(-1, -1);
static das::int2 select_mouse_pos = das::int2(-1, -1);

static void clear_selection()
{
  select_from_line = -1;
  select_to_line = -1;
  select_from_pos = -1;
  select_to_pos = -1;
  select_mouse_down = das::int2(-1, -1);
  select_mouse_pos = das::int2(-1, -1);
}

static void copy_log_to_clipboard()
{
  sf::String s;

  if (select_from_line < 0)
  {
    for (size_t i = 0; i < dasbox_logger.logStrings.size(); i++)
      s += dasbox_logger.logStrings[i];
  }
  else if (select_from_line == select_to_line)
  {
    if (select_from_pos > select_to_pos)
      std::swap(select_from_pos, select_to_pos);
    s = dasbox_logger.logStrings[select_from_line];
    s.replace("\t", "    ");
    if (int(s.getSize()) > select_from_pos)
      s = s.substring(select_from_pos, select_to_pos - select_from_pos + 1);
  }
  else
  {
    int ignoreLine = select_to_line;
    if (select_from_line > select_to_line)
      std::swap(select_from_line, select_to_line);
    for (int i = select_from_line; i <= select_to_line; i++)
      if (i != ignoreLine)
        s += dasbox_logger.logStrings[i];
  }

  sf::Clipboard::setString(s);
  clear_selection();
}


static void * last_key_pressed = nullptr;

struct KeyAutorepeat
{
  float timeToRepeat = 999.0f;
  bool currentPressed = false;
  bool triggered = false;

  void update(float dt, bool pressed)
  {
    if (pressed && !currentPressed)
    {
      last_key_pressed = this;
      timeToRepeat = KEY_AUTOREPEAT_START;
      triggered = true;
    }

    if (pressed && currentPressed)
    {
      timeToRepeat -= dt;
      if (timeToRepeat <= 0)
      {
        timeToRepeat += KEY_AUTOREPEAT_PERIOD;
        triggered = true;
      }
    }

    if (!pressed && currentPressed)
    {
      if (last_key_pressed && last_key_pressed == this)
        last_key_pressed = nullptr;
    }

    currentPressed = pressed;
  }

  bool fetchTrigger()
  {
    if (last_key_pressed && last_key_pressed != this)
      return false;

    bool v = triggered;
    triggered = false;
    return v;
  }
};


static das::int2 mouse_pos_to_log_pos(das::float2 mouse_pos, bool allow_negative_y = false)
{
  mouse_pos.x -= SAFE_AREA;
  mouse_pos.x /= FONT_WIDTH;
  mouse_pos.y -= 2 + FONT_HEIGHT * 3;
  mouse_pos.y /= FONT_HEIGHT;
  das::int2 res = das::int2(int(mouse_pos.x), int(mouse_pos.y));
  res.y += dasbox_logger.topLine;
  if (res.x < 0)
    res.x = 0;
  if (allow_negative_y)
  {
    if (res.y < -1)
      res.y = -1;
  }
  else
  {
    if (res.y < 0)
      res.y = 0;
  }
  if (res.y >= int(dasbox_logger.logStrings.size()))
    res.y = int(dasbox_logger.logStrings.size()) - 1;
  return res;
}


void update_log_screen(float dt)
{
  float scroll_delta = input::get_mouse_scroll_delta();
  scroll_accum += input::get_mouse_scroll_delta() * 4.0f;
  if (abs(int(scroll_accum)) >= 1)
  {
    dasbox_logger.topLine -= int(scroll_accum);
    scroll_accum -= int(scroll_accum);
  }

  if (fabsf(scroll_delta) < 0.001f)
  {
    time_to_reset_scroll_accum -= dt;
    if (time_to_reset_scroll_accum < 0)
      scroll_accum = 0.0;
  }
  else
    time_to_reset_scroll_accum = 2.0f;

  if (input::get_key_press(sf::Keyboard::Up))
    dasbox_logger.topLine--;

  if (input::get_key_press(sf::Keyboard::Down))
    dasbox_logger.topLine++;

  if (input::get_key_press(sf::Keyboard::PageUp))
  {
    dasbox_logger.topLine -= LOG_LINES_PER_SCREEN - 2;
    if (input::get_key(sf::Keyboard::LControl) || input::get_key(sf::Keyboard::RControl))
      dasbox_logger.topLine = 0;
  }

  if (input::get_key_press(sf::Keyboard::PageDown))
  {
    dasbox_logger.topLine += LOG_LINES_PER_SCREEN - 2;
    if (input::get_key(sf::Keyboard::LControl) || input::get_key(sf::Keyboard::RControl))
      dasbox_logger.topLine = INT_MAX;
  }

  if (input::get_key_down(sf::Keyboard::Home))
    dasbox_logger.topLine = 0;

  if (input::get_key_down(sf::Keyboard::End))
    dasbox_logger.topLine = INT_MAX;

  if (dasbox_logger.topLine > int(dasbox_logger.logStrings.size()) - LOG_LINES_PER_SCREEN + 2)
    dasbox_logger.topLine = int(dasbox_logger.logStrings.size()) - LOG_LINES_PER_SCREEN + 2;

  if (dasbox_logger.topLine < 0)
    dasbox_logger.topLine = 0;


  if (input::get_key_down(sf::Keyboard::C) &&
    (input::get_key(sf::Keyboard::LControl) || input::get_key(sf::Keyboard::RControl)))
  {
    copy_log_to_clipboard();
  }

  if (input::get_mouse_button_down(0))
  {
    das::int2 mp = mouse_pos_to_log_pos(input::get_mouse_position());
    if (mp.y >= dasbox_logger.topLine && dasbox_logger.logStrings.size() > 0)
    {
      mouse_down = true;
      select_from_line = select_to_line = mp.y;
      select_from_pos = select_to_pos = mp.x;
    }
  }

  if (input::get_mouse_button_up(0))
  {
    mouse_down = false;
    if (select_to_line == select_from_line && select_to_pos == select_from_pos)
      clear_selection();
  }

  if (mouse_down)
  {
    das::int2 mp = mouse_pos_to_log_pos(input::get_mouse_position(), true);
    select_to_line = mp.y;
    select_to_pos = mp.x;
  }
}


void disable_alpha_blend();
void set_font_size_i(int size);
int  get_font_size_i();
void text_out_i(int x, int y, const char * str, uint32_t color);
void fill_rect_i(int x, int y, int width, int height, uint32_t color);
void stash_font();
void restore_font();
void set_font_name(const char *);
void transform2d_push();
void transform2d_reset();
void transform2d_pop();



void draw_log_screen()
{
  transform2d_push();
  transform2d_reset();
  disable_alpha_blend();
  stash_font();
  set_font_name(nullptr);
  int savedFontSize = get_font_size_i();
  set_font_size_i(FONT_HEIGHT - 3);
  fill_rect_i(0, 0, screen_width, screen_height, 0xFF202020);
  fill_rect_i(0, 0, screen_width, FONT_HEIGHT * 3, 0xFFE0E0E0);

  if (has_fatal_errors)
    text_out_i(SAFE_AREA, SAFE_AREA, "Press \"F5\" or change and save source file to reload", 0);
  else
    text_out_i(SAFE_AREA, SAFE_AREA, "Press \"Tab\" to switch back to application", 0);
  text_out_i(SAFE_AREA, SAFE_AREA + FONT_HEIGHT, "Up, Down, [Ctrl+] PgUp, [Ctrl+] PgDown - scroll", 0);

  if (dasbox_logger.logStrings.size() >= LOG_LINES_PER_SCREEN)
  {
    float relativePos = float(dasbox_logger.topLine) / max(int(dasbox_logger.logStrings.size()) - LOG_LINES_PER_SCREEN + 2, 1);
    fill_rect_i(screen_width - 4, FONT_HEIGHT * 3 + int(relativePos * (screen_height - FONT_HEIGHT * 4)), 3, FONT_HEIGHT,
      SCROLL_POSITION_COLOR);
  }

  int y = 2 + FONT_HEIGHT * 3;
  for (size_t i = dasbox_logger.topLine; i < dasbox_logger.logStrings.size(); i++)
  {
    if (select_from_line >= 0 &&
      int(i) >= min(select_from_line, select_to_line) && int(i) <= max(select_from_line, select_to_line))
    {
      if (select_from_line == select_to_line)
      {
        fill_rect_i(SAFE_AREA + min(select_to_pos, select_from_pos) * FONT_WIDTH - 1, y,
          abs(select_to_pos - select_from_pos) * FONT_WIDTH + FONT_WIDTH + 1, FONT_HEIGHT, 0xFF676767);
      }
      else if (int(i) != select_to_line)
      {
        fill_rect_i(SAFE_AREA - 1, y, LOG_SYMBOLS_PER_SCREEN * FONT_WIDTH + 1, FONT_HEIGHT, 0xFF676767);
      }
    }

    text_out_i(SAFE_AREA, y, dasbox_logger.logStrings[i].c_str(), dasbox_logger.lineColors[i]);
    y += FONT_HEIGHT;
    if (y > screen_height)
      break;
  }

  restore_font();
  transform2d_pop();
  set_font_size_i(savedFontSize);
}


void on_return_from_log_screen()
{
  if (saved_cursor_hidden)
    input::hide_cursor();
  input::set_relative_mouse_mode(saved_relative_mode);
}


void on_switch_to_log_screen()
{
  saved_relative_mode = input::is_relative_mouse_mode();
  saved_cursor_hidden = input::is_cursor_hidden();
  input::set_relative_mouse_mode(false);
  mouse_down = false;
  clear_selection();
  input::reset_input();
  scroll_accum = 0.0f;
  time_to_reset_scroll_accum = 2.0f;

  if (dasbox_logger.topErrorLine >= 0)
  {
    dasbox_logger.topLine = dasbox_logger.topErrorLine;
    dasbox_logger.topErrorLine = -1;
  }
  else
    dasbox_logger.topLine = INT_MAX;

  if (dasbox_logger.topLine < 0)
    dasbox_logger.topLine = 0;

  if (dasbox_logger.topLine > int(dasbox_logger.logStrings.size()) - LOG_LINES_PER_SCREEN + 2)
    dasbox_logger.topLine = int(dasbox_logger.logStrings.size()) - LOG_LINES_PER_SCREEN + 2;
}
