#ifndef    SDLEXCEPTION_HH
# define   SDLEXCEPTION_HH

# include <core_utils/CoreException.hh>

namespace sdl {
  namespace core {

    class SdlException: public utils::CoreException {
      public:

        SdlException(const std::string& message,
                     const std::string& module,
                     const std::string& cause = std::string());

        virtual ~SdlException() = default;

      private:

        static const char* sk_serviceName;
    };

  }
}

# include "SdlException.hxx"

#endif    /* SDLEXCEPTION_HH */