#ifndef    FONT_HH
# define   FONT_HH

# include <memory>
# include <unordered_map>
# include <SDL2/SDL_ttf.h>
# include <core_utils/CoreObject.hh>
# include "Color.hh"

namespace sdl {
  namespace core {

    class Font: public utils::CoreObject {
      public:

        explicit
        Font(const std::string& name, const int& size = 25);

        explicit
        Font(const Font& other);

        virtual ~Font();

        const std::string&
        getName() const noexcept;

        const int&
        getSize() const noexcept;

        void
        setSize(const int& size) noexcept;

        SDL_Surface*
        render(const std::string& text, const Color& color);

      private:

        bool
        loaded() const noexcept;

        void
        load();

        void
        unload();

        void
        unloadAll();

        void
        initializeTTFLib();

      private:

        std::string m_name;
        int m_size;
        std::unordered_map<int, TTF_Font*> m_fonts;
        TTF_Font* m_font;

    };

    using FontShPtr = std::shared_ptr<Font>;
  }
}

# include "Font.hxx"

#endif    /* FONT_HH */
