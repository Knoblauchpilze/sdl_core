#ifndef    LAYOUT_EXCEPTION_HH
# define   LAYOUT_EXCEPTION_HH

# include "SdlException.hh"

namespace sdl {
  namespace core {

    class LayoutException: public SdlException {
      public:

        LayoutException(const std::string& message);

        virtual ~LayoutException() = default;

      private:

        static const char* sk_moduleName;
    };

  }
}

# include "LayoutException.hxx"

#endif    /* LAYOUT_EXCEPTION_HH */