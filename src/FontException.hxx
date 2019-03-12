#ifndef    FONTEXCEPTION_HXX
# define   FONTEXCEPTION_HXX

# include "FontException.hh"

namespace sdl {
  namespace core {

    inline
    FontException::FontException(const std::string& message):
      sdl::core::SdlException(message, sk_moduleName)
    {}

  }
}

#endif    /* FONTEXCEPTION_HXX */
