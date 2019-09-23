
# include "SizePolicy.hh"
# include <core_utils/CoreException.hh>

namespace sdl {
  namespace core {

    SizePolicy::SizePolicyFlag
    SizePolicy::initFromName(const Name& name) {
      bool valid = true;

      SizePolicyFlag out;

      switch (name) {
        case Name::Fixed:
          // Do nothing: neither growth nor shrink are desirable in this
          // strategy.
          break;
        case Name::Minimum:
          out |= SizePolicyFlag(size::Policy::Grow);
          break;
        case Name::Maximum:
          out |= SizePolicyFlag(size::Policy::Shrink);
          break;
        case Name::Preferred:
          out |= SizePolicyFlag(size::Policy::Grow);
          out |= SizePolicyFlag(size::Policy::Shrink);
          break;
        case Name::Expanding:
          out |= initFromName(Name::Preferred);
          out |= SizePolicyFlag(size::Policy::Expand);
          break;
        case Name::MinimumExpanding:
          out |= initFromName(Name::Minimum);
          out |= SizePolicyFlag(size::Policy::Expand);
          break;
        case Name::Ignored:
          out |= initFromName(Name::Preferred);
          out |= SizePolicyFlag(size::Policy::Ignore);
          break;
        default:
          valid = false;
          break;
      }

      if (!valid) {
        throw utils::CoreException(
          std::string("Could not init size policy flag from name ") + std::to_string(static_cast<int>(name)),
          std::string("initFromName"),
          std::string("SizePolicy"),
          std::string("Unhandled policy name")
        );
      }

      return out;
    }

  }
}
