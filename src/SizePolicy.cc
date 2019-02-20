
# include "SizePolicy.hh"

namespace {

  inline
  sdl::core::SizePolicy::Policy
  operator|(const sdl::core::SizePolicy::Policy& lhs, const sdl::core::SizePolicy::Policy& rhs) {
    return static_cast<sdl::core::SizePolicy::Policy>(static_cast<int>(lhs) | static_cast<int>(rhs));
  }

}

namespace sdl {
  namespace core {

    const SizePolicy::Policy SizePolicy::Fixed(SizePolicy::Policy::Grow);
    const SizePolicy::Policy SizePolicy::Minimum(SizePolicy::Policy::Grow);
    const SizePolicy::Policy SizePolicy::Maximum(SizePolicy::Policy::Shrink);
    const SizePolicy::Policy SizePolicy::Preferred(SizePolicy::Grow | SizePolicy::Policy::Shrink);
    const SizePolicy::Policy SizePolicy::Expanding(SizePolicy::Policy::Grow | SizePolicy::Policy::Shrink | SizePolicy::Policy::Expand);
    const SizePolicy::Policy SizePolicy::MinimumExpanding(SizePolicy::Policy::Grow | SizePolicy::Policy::Expand);
    const SizePolicy::Policy SizePolicy::Ignored(SizePolicy::Policy::Grow | SizePolicy::Policy::Shrink | SizePolicy::Policy::Ignore);

  }
}
