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
    SizePolicy::SizePolicyFlag::SizePolicyFlag():
      utils::CoreFlag<size::count>(std::string("size_policy_flag")),
      m_strategiesToIDs()
    {
      init();
    }

    inline
    SizePolicy::SizePolicyFlag::SizePolicyFlag(const size::Policy& policy):
      utils::CoreFlag<size::count>(std::string("size_policy_flag")),
      m_strategiesToIDs()
    {
      init();

      set(getBitID(policy));
    }

    inline
    bool
    SizePolicy::SizePolicyFlag::canShrink() const noexcept {
      return isSet(getBitID(size::Policy::Shrink));
    }

    inline
    bool
    SizePolicy::SizePolicyFlag::canGrow() const noexcept {
      return isSet(getBitID(size::Policy::Grow));
    }

    inline
    bool
    SizePolicy::SizePolicyFlag::canExpand() const noexcept {
      return isSet(getBitID(size::Policy::Expand));
    }

    inline
    bool
    SizePolicy::SizePolicyFlag::canExtend() const noexcept {
      return
        isSet(getBitID(size::Policy::Grow)) ||
        isSet(getBitID(size::Policy::Expand))
      ;
    }

    inline
    void
    SizePolicy::SizePolicyFlag::init() {
      // Register all size strategies.
      registerSizePolicy(size::Policy::Grow);
      registerSizePolicy(size::Policy::Expand);
      registerSizePolicy(size::Policy::Shrink);
      registerSizePolicy(size::Policy::Ignore);
    }

    inline
    int
    SizePolicy::SizePolicyFlag::getBitID(const size::Policy& policy) const {
      // Find the corresponding size strategy in the internal table.
      StrategiesTable::const_iterator it = m_strategiesToIDs.find(policy);

      // Check for errors.
      if (it == m_strategiesToIDs.cend()) {
        throw utils::CoreException(
          std::string("Could not get bit index for \"") + size::getNameFromPolicy(policy) + "\"",
          std::string("getBitID"),
          std::string("SizePolicyFlag"),
          std::string("No such bit registered")
        );
      }

      // Return the corresponding index.
      return it->second;
    }

    inline
    void
    SizePolicy::SizePolicyFlag::registerSizePolicy(const size::Policy& policy) {
      // Register the name corresponding to the input strategy with false value and default
      // value.
      int id = addNamedBit(size::getNameFromPolicy(policy), false, false);

      // Register the returned index to easily retrieve its value later on.
      m_strategiesToIDs[policy] = id;
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
    void
    SizePolicy::setHorizontalPolicy(const Name& policy) noexcept {
      m_hPolicy = initFromName(policy);
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
    void
    SizePolicy::setVerticalPolicy(const Name& policy) noexcept {
      m_vPolicy = initFromName(policy);
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
    SizePolicy::isFixedHorizontally() const noexcept {
      return !canShrinkHorizontally() && !canExtendHorizontally();
    }

    inline
    bool
    SizePolicy::canShrinkHorizontally() const noexcept {
      return m_hPolicy.canShrink();
    }

    inline
    bool
    SizePolicy::canGrowHorizontally() const noexcept {
      return m_hPolicy.canGrow();
    }

    inline
    bool
    SizePolicy::canExpandHorizontally() const noexcept {
      return m_hPolicy.canExpand();
    }

    inline
    bool
    SizePolicy::canExtendHorizontally() const noexcept {
      return m_hPolicy.canExtend();
    }

    inline
    bool
    SizePolicy::isFixedVertically() const noexcept {
      return !canShrinkVertically() && !canExtendVertically();
    }

    inline
    bool
    SizePolicy::canShrinkVertically() const noexcept {
      return m_vPolicy.canShrink();
    }

    inline
    bool
    SizePolicy::canGrowVertically() const noexcept {
      return m_vPolicy.canGrow();
    }

    inline
    bool
    SizePolicy::canExpandVertically() const noexcept {
      return m_vPolicy.canExpand();
    }

    inline
    bool
    SizePolicy::canExtendVertically() const noexcept {
      return m_vPolicy.canExtend();
    }

  }
}

#endif    /* SIZE_POLICY_HXX */
