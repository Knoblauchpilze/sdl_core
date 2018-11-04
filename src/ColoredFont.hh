#ifndef    COLOREDFONT_HH
# define   COLOREDFONT_HH

# include <memory>
# include "Font.hh"

namespace sdl {
  namespace core {

    class ColoredFont {
      public:

        explicit
        ColoredFont(FontShPtr font, const SDL_Color& color = SDL_Color{255, 255, 255, SDL_ALPHA_OPAQUE});

        explicit
        ColoredFont(const ColoredFont& other);

        ~ColoredFont() = default;

        SDL_Surface*
        render(const std::string& text);

      private:

        FontShPtr m_fontPtr;
        SDL_Color m_color;

    };

    using ColoredFontShPtr = std::shared_ptr<ColoredFont>;
  }
}

# include "ColoredFont.hxx"

#endif    /* COLOREDFONT_HH */
