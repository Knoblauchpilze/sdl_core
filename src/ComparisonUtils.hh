#ifndef    COMPARISONUTILS_HH
# define   COMPARISONUTILS_HH

# include <limits>

namespace sdl {
  namespace utils {

    template <typename DataType>
    bool
    fuzzyEqual(const DataType& value1,
               const DataType& value2,
               const DataType& epsilon = std::numeric_limits<DataType>::min());
  }
}

# include "ComparisonUtils.hxx"

#endif    /* COMPARISONUTILS_HH */
