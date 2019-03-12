#ifndef    SDLEXCEPTION_HXX
# define   SDLEXCEPTION_HXX

# include "SdlException.hh"

namespace sdl {
  namespace core {

    inline
    SdlException::SdlException(const std::string& message,
                               const std::string& module,
                               const std::string& cause):
      utils::CoreException(message, module, sk_serviceName, cause)
    {}

  }
}

#endif    /* SDLEXCEPTION_HXX */