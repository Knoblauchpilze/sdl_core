#ifndef    LAYOUT_EXCEPTION_HXX
# define   LAYOUT_EXCEPTION_HXX

# include "LayoutException.hh"

namespace sdl {
  namespace core {

    inline
    LayoutException::LayoutException(const std::string& message):
      SdlException(message, sk_moduleName)
    {}

  }
}

#endif    /* LAYOUT_EXCEPTION_HXX */
