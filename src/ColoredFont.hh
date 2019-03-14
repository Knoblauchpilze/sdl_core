#ifndef    COLOREDFONT_HH
# define   COLOREDFONT_HH

# include <memory>
# include <core_utils/CoreObject.hh>
# include "Font.hh"
# include "Color.hh"

namespace sdl {
  namespace core {

    class ColoredFont: public utils::CoreObject {
      public:

        explicit
        ColoredFont(FontShPtr font, const Color& color);

        virtual ~ColoredFont();

        FontShPtr
        getFont() const noexcept;

        const Color&
        getColor() const noexcept;

        void
        setSize(const int& size) noexcept;

        void
        setColor(const Color& color) noexcept;

        SDL_Texture*
        render(SDL_Renderer* renderer, const std::string& text);

      private:

        FontShPtr m_font;
        Color m_color;
        bool m_dirty;

        SDL_Texture* m_text;

    };

    using ColoredFontShPtr = std::shared_ptr<ColoredFont>;
  }
}

# include "ColoredFont.hxx"

#endif    /* COLOREDFONT_HH */
