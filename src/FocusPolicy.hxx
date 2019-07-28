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
      return std::string("[Policy: ") + getNameFromPolicy(*this) + "]";
    }

    inline
    std::string
    FocusPolicy::getNameFromPolicy(const FocusPolicy& policy) noexcept {
      std::string name;
      if (policy.canGrabHoverFocus()) {
        name += "Hover";
      }
      if (policy.canGrabHoverFocus()) {
        if (!name.empty()) {
          name += "|";
        }
        name += "Click";
      }
      if (policy.canGrabHoverFocus()) {
        if (!name.empty()) {
          name += "|";
        }
        name += "Tab";
      }
      if (policy.canGrabHoverFocus()) {
        if (!name.empty()) {
          name += "|";
        }
        name += "Wheel";
      }
      
      if (name.empty()) {
        name = "No focus";
      }

      return name;
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
