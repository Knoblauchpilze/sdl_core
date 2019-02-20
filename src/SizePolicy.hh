#ifndef    SIZEPOLICY_HH
# define   SIZEPOLICY_HH

# include <memory>
# include <vector>
# include "Color.hh"

namespace sdl {
  namespace core {

    class SizePolicy {
      public:

        SizePolicy();

        ~SizePolicy() = default;

      public:

        enum Policy {
          None   = 0,
          Grow   = 1 << 0,
          Expand = 1 << 1,
          Shrink = 1 << 2,
          Ignore = 1 << 3,
        };

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

      private:

        Policy m_hPolicy;
        float m_hStretch;
        Policy m_vPolicy;
        float m_vStretch;

    };

    using SizePolicyShPtr = std::shared_ptr<SizePolicy>;
  }
}

# include "SizePolicy.hxx"

#endif    /* SIZEPOLICY_HH */
