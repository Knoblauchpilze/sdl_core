#ifndef    SDLEXCEPTION_HXX
# define   SDLEXCEPTION_HXX

# include "SdlException.hh"

namespace sdl {
  namespace core {

    inline
    SdlException::SdlException(const std::string& message,
                               const std::string& module,
                               const std::string& cause):
      ::core::utils::CoreException(message, module, cause)
    {}

  }
}

#endif    /* SDLEXCEPTION_HXX */