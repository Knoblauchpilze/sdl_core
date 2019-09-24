#ifndef    FOCUS_POLICY_HXX
# define   FOCUS_POLICY_HXX

# include "FocusPolicy.hh"

namespace sdl {
  namespace core {

    namespace focus {

      inline
      std::string
      getNameFromType(const Type& type) noexcept {
        switch (type) {
          case Type::Hover:
            return std::string("Hover");
          case Type::Click:
            return std::string("Click");
          case Type::Tab:
            return std::string("Tab");
          case Type::Wheel:
            return std::string("Wheel");
          default:
            return std::string("Unknown");
        }
      }

    }

    inline
    FocusPolicy
    createFocusFromName(const focus::Name& name) noexcept {
      FocusPolicy p;

      switch (name) {
        case focus::Name::StrongFocus:
          p.set(focus::Type::Hover);
          p.set(focus::Type::Click);
          p.set(focus::Type::Tab);
          p.set(focus::Type::Wheel);
          break;
        default:
          // Unhandled name, keep the default focus policy.
          break;
      }

      return p;
    }

    inline
    bool
    canGrabHoverFocus(const FocusPolicy& policy) noexcept {
      return policy.isSet(focus::Type::Hover);
    }

    inline
    bool
    canGrabClickFocus(const FocusPolicy& policy) noexcept {
      return policy.isSet(focus::Type::Click);
    }

    inline
    bool
    canGrabTabFocus(const FocusPolicy& policy) noexcept {
      return policy.isSet(focus::Type::Tab);
    }

    inline
    bool
    canGrabWheelFocus(const FocusPolicy& policy) noexcept {
      return policy.isSet(focus::Type::Wheel);
    }

  }
}

#endif    /* FOCUS_POLICY_HXX */
