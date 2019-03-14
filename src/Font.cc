
# include "Font.hh"

namespace sdl {
  namespace core {

    Font::Font(const std::string& name, const int& size):
      utils::CoreObject(name),
      m_name(name),
      m_size(std::max(1, size)),
      m_fonts(),
      m_font(nullptr)
    {
      setService(std::string("font"));

      // Initialize the lib.
      initializeTTFLib();
    }

    Font::Font(const Font& other):
      utils::CoreObject(other.getName()),
      m_name(other.m_name),
      m_size(other.m_size),
      m_fonts(other.m_fonts),
      m_font(nullptr)
    {
      // Nothing to do.
    }

  }
}
