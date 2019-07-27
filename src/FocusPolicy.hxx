#ifndef    FOCUS_POLICY_HXX
# define   FOCUS_POLICY_HXX

# include "FocusPolicy.hh"

namespace sdl {
  namespace core {

    inline
    FocusPolicy::FocusPolicy():
      m_policy(FocusPolicy::NoFocus)
    {}

    inline
    FocusPolicy::FocusPolicy(const Policy& policy):
      m_policy(policy)
    {}

    inline
    const FocusPolicy::Policy&
    FocusPolicy::getPolicy() const noexcept {
      return m_policy;
    }

    inline
    void
    FocusPolicy::setPolicy(const Policy& policy) noexcept {
      m_policy = policy;
    }

    inline
    bool
    FocusPolicy::canGrabHoverFocus() const noexcept {
      return m_policy & Policy::Hover;
    }

    inline
    bool
    FocusPolicy::canGrabClickFocus() const noexcept {
      return m_policy & Policy::Click;
    }

    inline
    bool
    FocusPolicy::canGrabTabFocus() const noexcept {
      return m_policy & Policy::Tab;
    }

    inline
    bool
    FocusPolicy::canGrabWheelFocus() const noexcept {
      return m_policy & Policy::Wheel;
    }

    inline
    std::string
    FocusPolicy::toString() const noexcept {
      return std::string("[Policy: ") + getNameFromPolicy(m_policy) + "]";
    }

    inline
    std::string
    FocusPolicy::getNameFromPolicy(const Policy& policy) noexcept {
      switch (policy) {
        case None:
          return "No focus";
        case Hover:
          return "Hover";
        case Click:
          return "Click";
        case Tab:
          return "Tab";
        case Wheel:
          return "Wheel";
        default:
          return "Unknown";
      }
    }

  }
}

inline
std::ostream&
operator<<(const sdl::core::FocusPolicy& policy, std::ostream& out) noexcept {
  return operator<<(out, policy);
}

inline
std::ostream&
operator<<(std::ostream& out, const sdl::core::FocusPolicy& policy) noexcept {
  out << policy.toString();
  return out;
}

#endif    /* FOCUS_POLICY_HXX */
