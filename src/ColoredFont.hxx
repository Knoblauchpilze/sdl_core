#ifndef    COLOREDFONT_HXX
# define   COLOREDFONT_HXX

# include "ColoredFont.hh"

namespace sdl {
  namespace core {

    inline
    ColoredFont::ColoredFont(FontShPtr font, const Color& color):
      utils::CoreObject(font->getName()),
      m_font(font),
      m_color(color),
      m_dirty(true),
      m_text(nullptr)
    {
      setService(std::string("font"));
    }

    ColoredFont::~ColoredFont() {
      if (m_text != nullptr) {
        SDL_DestroyTexture(m_text);
      }
    }

    inline
    FontShPtr
    ColoredFont::getFont() const noexcept {
      return m_font;
    }

    inline
    const Color&
    ColoredFont::getColor() const noexcept {
      return m_color;
    }

    inline
    void
    ColoredFont::setSize(const int& size) noexcept {
      m_font->setSize(size);
      m_dirty = true;
    }

    inline
    void
    ColoredFont::setColor(const Color& color) noexcept {
      m_color = color;
      m_dirty = true;
    }

    inline
    SDL_Texture*
    ColoredFont::render(SDL_Renderer* renderer, const std::string& text) {
      if (m_dirty) {
        if (m_text != nullptr) {
          SDL_DestroyTexture(m_text);
          m_text = nullptr;
        }

        SDL_Surface* textAsSurface = m_font->render(text, m_color);
        if (textAsSurface == nullptr) {
          error(std::string("Could not render text \"") + text + "\" with font \"" + m_font->getName() + "\"");
        }

        m_text = SDL_CreateTextureFromSurface(renderer, textAsSurface);
        SDL_FreeSurface(textAsSurface);
        if (m_text == nullptr) {
          error(std::string("Could not create texture from surface for text \"") + text + "\" and font \"" + m_font->getName() + "\"");
        }

        SDL_SetTextureAlphaMod(m_text, m_color.a());

        m_dirty = false;
      }

      return m_text;
    }

  }
}

#endif    /* COLOREDFONT_HXX */
