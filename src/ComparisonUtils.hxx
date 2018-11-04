#ifndef    COMPARISONUTILS_HXX
# define   COMPARISONUTILS_HXX

# include "ComparisonUtils.hh"

namespace sdl {
  namespace utils {

    template <typename DataType>
    bool
    fuzzyEqual(const DataType& value1,
               const DataType& value2,
               const DataType& epsilon)
    {
      return std::abs(value1 - value2) < epsilon;
    }
  }
}

#endif    /* COMPARISONUTILS_HXX */
