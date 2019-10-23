#ifndef    SIZE_POLICY_HXX
# define   SIZE_POLICY_HXX

# include "SizePolicy.hh"

namespace sdl {
  namespace core {

    namespace size {

      inline
      std::string
      getNameFromPolicy(const Policy& policy) noexcept {
          switch (policy) {
            case Policy::Grow:
              return std::string("Grow");
            case Policy::Expand:
              return std::string("Expand");
            case Policy::Shrink:
              return std::string("Shrink");
            case Policy::Ignore:
              return std::string("Ignore");
            default:
              return std::string("Unknown");
          }
      }

    }

    inline
    SizePolicy::SizePolicy(const Name& hPolicy,
                           const Name& vPolicy):
      m_hPolicy(initFromName(hPolicy)),
      m_hStretch(0.0f),
      m_vPolicy(initFromName(vPolicy)),
      m_vStretch(0.0f)
    {}

    inline
    bool
    SizePolicy::operator==(const SizePolicy& rhs) const noexcept {
      return
        m_hPolicy == rhs.m_hPolicy &&
        m_vPolicy == rhs.m_vPolicy &&
        m_hStretch == rhs.m_hStretch &&
        m_vStretch == rhs.m_vStretch
      ;
    }

    inline
    bool
    SizePolicy::operator!=(const SizePolicy& rhs) const noexcept {
      return !operator==(rhs);
    }

    inline
    void
    SizePolicy::setHorizontalPolicy(const Name& policy) noexcept {
      m_hPolicy = initFromName(policy);
    }

    inline
    float
    SizePolicy::getHorizontalStretch() const noexcept {
      return m_hStretch;
    }

    inline
    void
    SizePolicy::setHorizontalStretch(float stretch) noexcept {
      m_hStretch = stretch;
    }

    inline
    void
    SizePolicy::setVerticalPolicy(const Name& policy) noexcept {
      m_vPolicy = initFromName(policy);
    }

    inline
    float
    SizePolicy::getVerticalStretch() const noexcept {
      return m_vStretch;
    }

    inline
    void
    SizePolicy::setVerticalStretch(float stretch) noexcept {
      m_vStretch = stretch;
    }

    inline
    bool
    SizePolicy::isFixedHorizontally() const noexcept {
      return !canShrinkHorizontally() && !canExtendHorizontally();
    }

    inline
    bool
    SizePolicy::canShrinkHorizontally() const noexcept {
      return m_hPolicy.isSet(size::Policy::Shrink);
    }

    inline
    bool
    SizePolicy::canGrowHorizontally() const noexcept {
      return m_hPolicy.isSet(size::Policy::Grow);
    }

    inline
    bool
    SizePolicy::canExpandHorizontally() const noexcept {
      return m_hPolicy.isSet(size::Policy::Expand);
    }

    inline
    bool
    SizePolicy::canExtendHorizontally() const noexcept {
      return canGrowHorizontally() || canExpandHorizontally();
    }

    inline
    bool
    SizePolicy::isFixedVertically() const noexcept {
      return !canShrinkVertically() && !canExtendVertically();
    }

    inline
    bool
    SizePolicy::canShrinkVertically() const noexcept {
      return m_vPolicy.isSet(size::Policy::Shrink);
    }

    inline
    bool
    SizePolicy::canGrowVertically() const noexcept {
      return m_vPolicy.isSet(size::Policy::Grow);
    }

    inline
    bool
    SizePolicy::canExpandVertically() const noexcept {
      return m_vPolicy.isSet(size::Policy::Expand);
    }

    inline
    bool
    SizePolicy::canExtendVertically() const noexcept {
      return canGrowVertically() || canExpandVertically();
    }

  }
}

#endif    /* SIZE_POLICY_HXX */
