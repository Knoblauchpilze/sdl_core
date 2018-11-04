#ifndef    SDLEXCEPTION_HH
# define   SDLEXCEPTION_HH

# include <stdexcept>

namespace sdl {
  namespace core {

    class SdlException: public std::exception {
      public:

        SdlException(const std::string& cause);

        virtual ~SdlException() = default;

        const char*
        what() const throw() override;

      protected:

      private:

        std::string m_cause;
    };

  }
}

# include "SdlException.hxx"

#endif    /* SDLEXCEPTION_HH */