#ifndef    SIZE_POLICY_HH
# define   SIZE_POLICY_HH

# include <memory>

namespace sdl {
  namespace core {

    class SizePolicy {
      public:

        enum Policy {
          None   = 0,
          Grow   = 1 << 0,
          Expand = 1 << 1,
          Shrink = 1 << 2,
          Ignore = 1 << 3,
        };

      public:

        SizePolicy();

        SizePolicy(const Policy& hPolicy,
                   const Policy& vPolicy);

        ~SizePolicy() = default;

        static const Policy Fixed;
        static const Policy Minimum;
        static const Policy Maximum;
        static const Policy Preferred;
        static const Policy Expanding;
        static const Policy MinimumExpanding;
        static const Policy Ignored;

      public:

        const Policy&
        getHorizontalPolicy() const noexcept;

        void
        setHorizontalPolicy(const Policy& policy) noexcept;

        const float&
        getHorizontalStretch() const noexcept;

        void
        setHorizontalStretch(const float& stretch) noexcept;

        const Policy&
        getVerticalPolicy() const noexcept;

        void
        setVerticalPolicy(const Policy& policy) noexcept;

        const float&
        getVerticalStretch() const noexcept;

        void
        setVerticalStretch(const float& stretch) noexcept;

        bool
        canShrinkHorizontally() const noexcept;

        bool
        canGrowHorizontally() const noexcept;

        bool
        canExpandHorizontally() const noexcept;

        bool
        canExtendHorizontally() const noexcept;

        bool
        canShrinkVertically() const noexcept;

        bool
        canGrowVertically() const noexcept;

        bool
        canExpandVertically() const noexcept;

        bool
        canExtendVertically() const noexcept;

        std::string
        toString() const noexcept;

      private:

        static
        std::string
        getNameFromPolicy(const Policy& policy) noexcept;

      private:

        Policy m_hPolicy;
        float m_hStretch;
        Policy m_vPolicy;
        float m_vStretch;

    };

    using SizePolicyShPtr = std::shared_ptr<SizePolicy>;
  }
}

std::ostream&
operator<<(const sdl::core::SizePolicy& policy, std::ostream& out) noexcept;

std::ostream&
operator<<(std::ostream& out, const sdl::core::SizePolicy& policy) noexcept;

# include "SizePolicy.hxx"

#endif    /* SIZE_POLICY_HH */
