#ifndef    PALETTE_HXX
# define   PALETTE_HXX

# include "Palette.hh"

namespace sdl {
  namespace core {

    inline
    Palette::Palette():
      m_foregroundColor(0, 0, 0),
      m_backgroundColor(0, 128, 0),
      m_hoverColor(192, 192, 192),
      m_selectionColor(255, 255, 255),
      m_textColor(0, 0, 128),
      m_selectedTextColor(0, 0, 192)
    {}

    inline
    Color
    Palette::getForegroundColor() const noexcept {
      return m_foregroundColor;
    }

    inline
    void
    Palette::setForegroundColor(const Color& color) {
      m_foregroundColor = color;
    }

    inline
    Color
    Palette::getBackgroundColor() const noexcept {
      return m_backgroundColor;
    }

    inline
    void
    Palette::setBackgroundColor(const Color& color) {
      m_backgroundColor = color;
    }

    inline
    Color
    Palette::getHoverColor() const noexcept {
      return m_hoverColor;
    }

    inline
    void
    Palette::setHoverColor(const Color& color) {
      m_hoverColor = color;
    }

    inline
    Color
    Palette::getSelectionColor() const noexcept {
      return m_selectionColor;
    }

    inline
    void
    Palette::setSelectionColor(const Color& color) {
      m_selectionColor = color;
    }

    inline
    Color
    Palette::getTextColor() const noexcept {
      return m_textColor;
    }

    inline
    void
    Palette::setTextColor(const Color& color) {
      m_textColor = color;
    }

    inline
    Color
    Palette::getSelectedTextColor() const noexcept {
      return m_selectedTextColor;
    }

    inline
    void
    Palette::setSelectedTextColor(const Color& color) {
      m_selectedTextColor = color;
    }

    inline
    Palette
    Palette::fromBackgroundColor(const Color& color) noexcept {
      Palette palette;
      palette.m_backgroundColor = color;

      palette.m_foregroundColor = palette.m_backgroundColor.brighten(1.5f);
      palette.m_hoverColor = Color::CorneFlowerBlue;
      palette.m_selectionColor = palette.m_hoverColor;
      palette.m_textColor = (palette.m_backgroundColor.brightness() < 0.5f ? Color::White : Color::Black);
      palette.m_selectedTextColor = palette.m_textColor;

      return palette;
    }

  }
}

#endif    /* PALETTE_HXX */
