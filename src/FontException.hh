#ifndef    FONTEXCEPTION_HH
# define   FONTEXCEPTION_HH

# include "SdlException.hh"

namespace sdl {
  namespace core {

    class FontException: public SdlException {
      public:

        FontException(const std::string& cause);

        virtual ~FontException() = default;

      private:

        static const char* sk_moduleName;
    };

  }
}

# include "FontException.hxx"

#endif    /* FONTEXCEPTION_HH */