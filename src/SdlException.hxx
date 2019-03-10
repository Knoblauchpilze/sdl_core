#ifndef    SDLEXCEPTION_HXX
# define   SDLEXCEPTION_HXX

# include "SdlException.hh"

namespace sdl {
  namespace core {

    inline
    SdlException::SdlException(const std::string& message,
                               const std::string& cause):
      utils::core::CoreException(message, sk_moduleName, cause)
    {}

  }
}

#endif    /* SDLEXCEPTION_HXX */