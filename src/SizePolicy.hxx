#ifndef    SIZE_POLICY_HXX
# define   SIZE_POLICY_HXX

# include "SizePolicy.hh"

namespace sdl {
  namespace core {

    inline
    SizePolicy::SizePolicy():
      m_hPolicy(SizePolicy::Fixed),
      m_hStretch(0),
      m_vPolicy(SizePolicy::Fixed),
      m_vStretch(0)
    {}

    inline
    SizePolicy::SizePolicy(const Policy& hPolicy,
                           const Policy& vPolicy):
      m_hPolicy(hPolicy),
      m_hStretch(0),
      m_vPolicy(vPolicy),
      m_vStretch(0)
    {}

    inline
    const SizePolicy::Policy&
    SizePolicy::getHorizontalPolicy() const noexcept {
      return m_hPolicy;
    }

    inline
    void
    SizePolicy::setHorizontalPolicy(const Policy& policy) noexcept {
      m_hPolicy = policy;
    }

    inline
    const float&
    SizePolicy::getHorizontalStretch() const noexcept {
      return m_hStretch;
    }

    inline
    void
    SizePolicy::setHorizontalStretch(const float& stretch) noexcept {
      m_hStretch = stretch;
    }

    inline
    const SizePolicy::Policy&
    SizePolicy::getVerticalPolicy() const noexcept {
      return m_vPolicy;
    }

    inline
    void
    SizePolicy::setVerticalPolicy(const Policy& policy) noexcept {
      m_vPolicy = policy;
    }

    inline
    const float&
    SizePolicy::getVerticalStretch() const noexcept {
      return m_vStretch;
    }

    inline
    void
    SizePolicy::setVerticalStretch(const float& stretch) noexcept {
      m_vStretch = stretch;
    }

    inline
    bool
    SizePolicy::canShrinkHorizontally() const noexcept {
      return m_hPolicy & Policy::Shrink;
    }

    inline
    bool
    SizePolicy::canGrowHorizontally() const noexcept {
      return m_hPolicy & Policy::Grow;
    }

    inline
    bool
    SizePolicy::canExpandHorizontally() const noexcept {
      return m_hPolicy & Policy::Expand;
    }

    inline
    bool
    SizePolicy::canExtendHorizontally() const noexcept {
      return canGrowHorizontally() || canExpandHorizontally();
    }

    inline
    bool
    SizePolicy::canShrinkVertically() const noexcept {
      return m_vPolicy & Policy::Shrink;
    }

    inline
    bool
    SizePolicy::canGrowVertically() const noexcept {
      return m_vPolicy & Policy::Grow;
    }

    inline
    bool
    SizePolicy::canExpandVertically() const noexcept {
      return m_vPolicy & Policy::Expand;
    }

    inline
    bool
    SizePolicy::canExtendVertically() const noexcept {
      return canGrowVertically() || canExpandVertically();
    }

    inline
    std::string
    SizePolicy::toString() const noexcept {
      return std::string("[Policy: h=") + getNameFromPolicy(m_hPolicy) + ", v=" + getNameFromPolicy(m_vPolicy) + "]";
    }

    inline
    std::string
    SizePolicy::getNameFromPolicy(const Policy& policy) noexcept {
      switch (policy) {
        case None:
          return "None";
        case Grow:
          return "Grow";
        case Expand:
          return "Expand";
        case Shrink:
          return "Shrink";
        case Ignore:
          return "Ignore";
        default:
          return "Unknown";
      }
    }

  }
}

inline
std::ostream&
operator<<(const sdl::core::SizePolicy& policy, std::ostream& out) noexcept {
  return operator<<(out, policy);
}

inline
std::ostream&
operator<<(std::ostream& out, const sdl::core::SizePolicy& policy) noexcept {
  out << policy.toString();
  return out;
}

#endif    /* SIZE_POLICY_HXX */
