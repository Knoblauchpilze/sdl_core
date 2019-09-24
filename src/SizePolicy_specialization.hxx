#ifndef    SIZE_POLICY_SPECIALIZATION_HXX
# define   SIZE_POLICY_SPECIALIZATION_HXX

# include "SizePolicy.hh"

namespace utils {

  template <>
  inline
  std::string
  getNameForKey(const sdl::core::size::Policy& policy) {
    return sdl::core::size::getNameFromPolicy(policy);
  }

}

#endif    /* SIZE_POLICY_SPECIALIZATION_HXX */
