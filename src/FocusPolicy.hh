#ifndef    FOCUS_POLICY_HH
# define   FOCUS_POLICY_HH

# include <memory>

namespace sdl {
  namespace core {

    class FocusPolicy {
      public:

        enum Policy {
          None = 0,
          Hover   = 1 << 0,
          Click   = 1 << 1,
          Tab     = 1 << 2,
          Wheel   = 1 << 3,
        };

      public:

        FocusPolicy();

        FocusPolicy(const Policy& policy);

        ~FocusPolicy() = default;

        static const Policy NoFocus;
        static const Policy HoverFocus;
        static const Policy ClickFocus;
        static const Policy TabFocus;
        static const Policy WheelFocus;
        static const Policy StrongFocus;

      public:

        const Policy&
        getPolicy() const noexcept;

        void
        setPolicy(const Policy& policy) noexcept;

        bool
        canGrabHoverFocus() const noexcept;

        bool
        canGrabClickFocus() const noexcept;

        bool
        canGrabTabFocus() const noexcept;

        bool
        canGrabWheelFocus() const noexcept;

        std::string
        toString() const noexcept;

      private:

        static
        std::string
        getNameFromPolicy(const FocusPolicy& policy) noexcept;

      private:

        Policy m_policy;

    };

    using FocusPolicyShPtr = std::shared_ptr<FocusPolicy>;
  }
}

std::ostream&
operator<<(const sdl::core::FocusPolicy& policy, std::ostream& out) noexcept;

std::ostream&
operator<<(std::ostream& out, const sdl::core::FocusPolicy& policy) noexcept;

# include "FocusPolicy.hxx"

#endif    /* FOCUS_POLICY_HH */
