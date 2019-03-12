#ifndef    FONTFACTORY_HXX
# define   FONTFACTORY_HXX

# include "FontFactory.hh"

namespace sdl {
  namespace core {

    FontFactory& FontFactory::getInstance() {
      static FontFactory instance;
      return instance;
    }

    void FontFactory::releaseFonts() {
      m_fonts.clear();
    }

    inline
    FontShPtr
    FontFactory::createFont(const std::string& name, const int size) {
      // Try to find this couple.
      std::unordered_map<std::string, FontShPtr>::const_iterator font = m_fonts.find(name);
      if (font != m_fonts.cend()) {
        return font->second;
      }

      // TODO: INvalid size usage: if several client share the same font pointer, we
      // are not able to perform the rendering with the desired size.
      // A better approach would be to effectively duplicate the font but to share the
      // array of sizes.
      m_fonts[name] = std::make_shared<Font>(name, size);

      return m_fonts[name];
    }

    inline
    ColoredFontShPtr
    FontFactory::createColoredFont(const std::string& name,
                                   const Color& color,
                                   const int size)
    {
      return std::make_shared<ColoredFont>(createFont(name, size), color);
    }

  }
}

#endif    /* FONTFACTORY_HXX */
