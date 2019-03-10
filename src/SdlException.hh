#ifndef    SDLEXCEPTION_HH
# define   SDLEXCEPTION_HH

# include <core_utils/CoreException.hh>

namespace sdl {
  namespace core {

    class SdlException: public ::core::utils::CoreException {
      public:

        SdlException(const std::string& message,
                     const std::string& module = std::string("sdl"),
                     const std::string& cause = std::string());

        virtual ~SdlException() = default;
    };

  }
}

# include "SdlException.hxx"

#endif    /* SDLEXCEPTION_HH */