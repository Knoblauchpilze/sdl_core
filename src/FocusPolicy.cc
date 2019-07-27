
# include "FocusPolicy.hh"

namespace {

  inline
  sdl::core::FocusPolicy::Policy
  operator|(const sdl::core::FocusPolicy::Policy& lhs, const sdl::core::FocusPolicy::Policy& rhs) {
    return static_cast<sdl::core::FocusPolicy::Policy>(static_cast<int>(lhs) | static_cast<int>(rhs));
  }

}

namespace sdl {
  namespace core {

    const FocusPolicy::Policy FocusPolicy::NoFocus(FocusPolicy::Policy::None);
    const FocusPolicy::Policy FocusPolicy::HoverFocus(FocusPolicy::Policy::Hover);
    const FocusPolicy::Policy FocusPolicy::ClickFocus(FocusPolicy::Policy::Click);
    const FocusPolicy::Policy FocusPolicy::TabFocus(FocusPolicy::Policy::Tab);
    const FocusPolicy::Policy FocusPolicy::WheelFocus(FocusPolicy::Policy::Wheel);
    const FocusPolicy::Policy FocusPolicy::StrongFocus(FocusPolicy::Policy::Hover | FocusPolicy::Policy::Click | FocusPolicy::Policy::Tab | FocusPolicy::Policy::Wheel);

  }
}
