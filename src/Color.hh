#ifndef    COLOR_HH
# define   COLOR_HH

# include <memory>
# include <SDL2/SDL.h>

namespace sdl {
  namespace core {

    class Color {
      public:

        Color(const SDL_Color& color);

        Color(const uint8_t& r,
              const uint8_t& g,
              const uint8_t& b,
              const uint8_t& a = SDL_ALPHA_OPAQUE);

        ~Color() = default;

        SDL_Color
        operator()() const noexcept;

        uint8_t
        r() const noexcept;

        uint8_t
        g() const noexcept;

        uint8_t
        b() const noexcept;

        uint8_t
        a() const noexcept;

        bool
        isOpaque() const noexcept;

        bool
        isTransparent() const noexcept;

        float
        brightness() const noexcept;

      private:

        SDL_Color m_color;

    };

    using ColorShPtr = std::shared_ptr<Color>;
  }
}

# include "Color.hxx"

#endif    /* COLOR_HH */
