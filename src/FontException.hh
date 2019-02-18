#ifndef    FONTEXCEPTION_HH
# define   FONTEXCEPTION_HH

# include "SdlException.hh"

namespace sdl {
  namespace core {

    class FontException: public SdlException {
      public:

        FontException(const std::string& cause);

        virtual ~FontException() = default;
    };

  }
}

# include "FontException.hxx"

#endif    /* FONTEXCEPTION_HH */