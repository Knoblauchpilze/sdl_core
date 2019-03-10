#ifndef    COLOR_HH
# define   COLOR_HH

# include <memory>
# include <SDL2/SDL.h>

namespace sdl {
  namespace core {

    class Color {
      public:

        Color();

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

        Color
        brighten(const float factor) noexcept;

        Color
        darken(const float factor) noexcept;

      public:

        static const Color White;
        static const Color Black;
        static const Color Red;
        static const Color Green;
        static const Color Blue;
        static const Color Yellow;
        static const Color Orange;
        static const Color Cyan;
        static const Color Magenta;
        static const Color Silver;
        static const Color Gray;
        static const Color Maroon;
        static const Color Olive;
        static const Color Purple;
        static const Color Teal;
        static const Color Navy;
        static const Color CorneFlowerBlue;

      private:

        SDL_Color m_color;

    };

    using ColorShPtr = std::shared_ptr<Color>;
  }
}

# include "Color.hxx"

#endif    /* COLOR_HH */
