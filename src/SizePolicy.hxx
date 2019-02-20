#ifndef    SIZEPOLICY_HXX
# define   SIZEPOLICY_HXX

# include "SizePolicy.hh"

namespace sdl {
  namespace core {

    inline
    SizePolicy::SizePolicy():
      m_hPolicy(SizePolicy::Fixed),
      m_hStretch(1),
      m_vPolicy(SizePolicy::Fixed),
      m_vStretch(1)
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

  }
}

#endif    /* SIZEPOLICY_HXX */
