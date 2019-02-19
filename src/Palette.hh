#ifndef    PALETTE_HH
# define   PALETTE_HH

# include <memory>
# include <vector>
# include "Color.hh"

namespace sdl {
  namespace core {

    class Palette {
      public:

        Palette();

        ~Palette() = default;

        Color
        getForegroundColor() const noexcept;

        void
        setForegroundColor(const Color& color);

        Color
        getBackgroundColor() const noexcept;

        void
        setBackgroundColor(const Color& color);

        Color
        getHoverColor() const noexcept;

        void
        setHoverColor(const Color& color);

        Color
        getSelectionColor() const noexcept;

        void
        setSelectionColor(const Color& color);

        Color
        getTextColor() const noexcept;

        void
        setTextColor(const Color& color);

        Color
        getSelectedTextColor() const noexcept;

        void
        setSelectedTextColor(const Color& color);

        static
        Palette
        fromBackgroundColor(const Color& color) noexcept;

      private:

        Color m_foregroundColor;
        Color m_backgroundColor;
        Color m_hoverColor;
        Color m_selectionColor;
        Color m_textColor;
        Color m_selectedTextColor;

    };

    using PaletteShPtr = std::shared_ptr<Palette>;
  }
}

# include "Palette.hxx"

#endif    /* PALETTE_HH */
