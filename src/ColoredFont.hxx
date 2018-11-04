#ifndef    COLOREDFONT_HXX
# define   COLOREDFONT_HXX

#include "ColoredFont.hh"

namespace sdl {
  namespace core {

    inline
    ColoredFont::ColoredFont(FontShPtr font, const SDL_Color& color):
      m_fontPtr(font),
      m_color(color)
    {}

    inline
    ColoredFont::ColoredFont(const ColoredFont& other):
      m_fontPtr(other.m_fontPtr),
      m_color(other.m_color)
    {}

    inline
    SDL_Surface*
    ColoredFont::render(const std::string& text) {
      return m_fontPtr->render(text, m_color);
    }

  }
}

#endif    /* COLOREDFONT_HXX */
