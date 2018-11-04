#ifndef    SDLEXCEPTION_HXX
# define   SDLEXCEPTION_HXX

# include "SdlException.hh"

namespace sdl {
  namespace core {

    inline
    SdlException::SdlException(const std::string& cause):
      std::exception(),
      m_cause(cause)
    {}

    inline
    const char*
    SdlException::what() const throw() {
      return m_cause.c_str();
    }

  }
}

#endif    /* SDLEXCEPTION_HXX */