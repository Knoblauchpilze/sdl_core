#ifndef    BOX_UTILS_HXX
# define   BOX_UTILS_HXX

# include "BoxUtils.hh"

namespace utils {
  namespace sdl {

    template <typename CoordinateType>
    inline
    SDL_Rect
    toSDLRect(const utils::maths::Box<CoordinateType>& box) noexcept {
      return SDL_Rect{
        static_cast<int>(box.getLeftBound()),
        static_cast<int>(box.getBottomBound()),
        static_cast<int>(box.w()),
        static_cast<int>(box.h())
      };
    }

  }
}

#endif    /* BOX_UTILS_HXX */
