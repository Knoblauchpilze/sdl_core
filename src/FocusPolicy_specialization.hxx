#ifndef    FOCUS_POLICY_SPECIALIZATION_HXX
# define   FOCUS_POLICY_SPECIALIZATION_HXX

# include "FocusPolicy.hh"

namespace utils {

  template <>
  inline
  std::string
  getNameForKey(const sdl::core::focus::Type& type) {
    return sdl::core::focus::getNameFromType(type);
  }

}

#endif    /* FOCUS_POLICY_SPECIALIZATION_HXX */
