#ifndef    COLOR_HXX
# define   COLOR_HXX

# include "Color.hh"

namespace sdl {
  namespace core {

    inline
    Color::Color():
      m_color(SDL_Color{0, 0, 0, SDL_ALPHA_OPAQUE})
    {}

    inline
    Color::Color(const SDL_Color& color):
      m_color(color)
    {}

    inline
    Color::Color(const uint8_t& r,
                 const uint8_t& g,
                 const uint8_t& b,
                 const uint8_t& a):
      m_color(SDL_Color{r, g, b, a})
    {}

    inline
    SDL_Color
    Color::operator()() const noexcept {
      return m_color;
    }

    inline
    uint8_t
    Color::r() const noexcept {
      return m_color.r;
    }

    inline
    uint8_t
    Color::g() const noexcept {
      return m_color.g;
    }

    inline
    uint8_t
    Color::b() const noexcept {
      return m_color.b;
    }

    inline
    uint8_t
    Color::a() const noexcept {
      return m_color.a;
    }

    inline
    bool
    Color::isOpaque() const noexcept {
      return m_color.a == SDL_ALPHA_OPAQUE;
    }

    inline
    bool
    Color::isTransparent() const noexcept {
      return m_color.a != SDL_ALPHA_OPAQUE;
    }

    inline
    float
    Color::brightness() const noexcept {
      return 0.299f * static_cast<float>(m_color.r) +
             0.587f * static_cast<float>(m_color.g) +
             0.114f * static_cast<float>(m_color.b);
    }

    inline
    Color
    Color::brighten(const float factor) noexcept {
      const uint8_t r = static_cast<uint8_t>(std::min(255.0f, static_cast<float>(m_color.r) * factor));
      const uint8_t g = static_cast<uint8_t>(std::min(255.0f, static_cast<float>(m_color.g) * factor));
      const uint8_t b = static_cast<uint8_t>(std::min(255.0f, static_cast<float>(m_color.b) * factor));
      return Color(r, g, b, m_color.a);
    }

    inline
    Color
    Color::darken(const float factor) noexcept {
      const uint8_t r = static_cast<uint8_t>(std::min(255.0f, static_cast<float>(m_color.r) / factor));
      const uint8_t g = static_cast<uint8_t>(std::min(255.0f, static_cast<float>(m_color.g) / factor));
      const uint8_t b = static_cast<uint8_t>(std::min(255.0f, static_cast<float>(m_color.b) / factor));
      return Color(r, g, b, m_color.a);
    }

  }
}

#endif    /* COLOR_HXX */
