#ifndef    BOX_HXX_INCLUDED
# define   BOX_HXX_INCLUDED

# include "Box.hh"

namespace sdl {
  namespace utils {

    template <typename CoordinateType>
    inline
    Box<CoordinateType>::Box(const CoordinateType& x,
                             const CoordinateType& y,
                             const CoordinateType& width,
                             const CoordinateType& height) noexcept :
      m_x(x),
      m_y(y),
      m_w(width),
      m_h(height)
    {}

    template <typename CoordinateType>
    inline
    CoordinateType&
    Box<CoordinateType>::x() noexcept {
      return m_x;
    } 

    template <typename CoordinateType>
    inline
    const CoordinateType&
    Box<CoordinateType>::x() const noexcept {
      return m_x;
    }

    template <typename CoordinateType>
    inline
    CoordinateType&
    Box<CoordinateType>::y() noexcept {
      return m_y;
    }

    template <typename CoordinateType>
    inline
    const CoordinateType&
    Box<CoordinateType>::y() const noexcept {
      return m_y;
    }
    
    template <typename CoordinateType>
    inline
    CoordinateType&
    Box<CoordinateType>::w() noexcept {
      return m_w;
    }

    template <typename CoordinateType>
    inline
    const CoordinateType&
    Box<CoordinateType>::w() const noexcept {
      return m_w;
    }

    template <typename CoordinateType>
    inline
    CoordinateType&
    Box<CoordinateType>::h() noexcept {
      return m_h;
    }

    template <typename CoordinateType>
    inline
    const CoordinateType&
    Box<CoordinateType>::h() const noexcept {
      return m_h;
    }

    template <typename CoordinateType>
    inline
    CoordinateType
    Box<CoordinateType>::getLeftBound() const noexcept {
      return m_x - m_w / CoordinateType(2.0f);
    }

    template <typename CoordinateType>
    inline
    CoordinateType
    Box<CoordinateType>::getRightBound() const noexcept {
      return m_x + m_w / CoordinateType(2.0f);
    }

    template <typename CoordinateType>
    inline
    CoordinateType
    Box<CoordinateType>::getTopBound() const noexcept {
      return m_y + m_h / CoordinateType(2.0f);
    }

    template <typename CoordinateType>
    inline
    CoordinateType
    Box<CoordinateType>::getBottomBound() const noexcept {
      return m_y - m_h / CoordinateType(2.0f);
    }

    template <typename CoordinateType>
    inline
    Vector2<CoordinateType>
    Box<CoordinateType>::getCenter() const noexcept {
        return Vector2<CoordinateType>(m_x, m_y);
    }

    template <typename CoordinateType>
    inline
    Vector2<CoordinateType>
    Box<CoordinateType>::getTopLeftCorner() const noexcept {
        return Vector2<CoordinateType>(m_x - m_w / CoordinateType(2.0f), m_y + m_h / CoordinateType(2.0f));
    }

    template <typename CoordinateType>
    inline
    Vector2<CoordinateType>
    Box<CoordinateType>::getTopRightCorner() const noexcept {
        return Vector2<CoordinateType>(m_x + m_w / CoordinateType(2.0f), m_y + m_h / CoordinateType(2.0f));
    }

    template <typename CoordinateType>
    inline
    Vector2<CoordinateType>
    Box<CoordinateType>::getBottomRightCorner() const noexcept {
        return Vector2<CoordinateType>(m_x + m_w / CoordinateType(2.0f), m_y - m_h / CoordinateType(2.0f));
    }

    template <typename CoordinateType>
    inline
    Vector2<CoordinateType>
    Box<CoordinateType>::getBottomLeftCorner() const noexcept {
        return Vector2<CoordinateType>(m_x - m_w / CoordinateType(2.0f), m_y - m_h / CoordinateType(2.0f));
    }

    template <typename CoordinateType>
    inline
    CoordinateType
    Box<CoordinateType>::getSurface() const noexcept {
      return m_w * m_h;
    }

    template <typename CoordinateType>
    inline
    bool
    Box<CoordinateType>::contains(const Box<CoordinateType>& other) const noexcept {
      return other.getLeftBound() >=  getLeftBound() &&
             other.getRightBound() <= getRightBound() &&
             other.getTopBound() <= getTopBound() &&
             other.getBottomBound() >= getBottomBound();
    }

    template <typename CoordinateType>
    inline
    bool
    Box<CoordinateType>::intersects(const Box<CoordinateType>& other) const noexcept {
      return getLeftBound() <= other.getRightBound() ||
             getRightBound() >= other.getLeftBound() ||
             getTopBound() >= other.getBottomBound() ||
             getBottomBound() <= other.getTopBound();
    }

    template <typename CoordinateType>
    inline
    bool
    Box<CoordinateType>::isInside(const Box<CoordinateType>& other) const noexcept {
      return other.contains(*this);
    }

    template <typename CoordinateType>
    inline
    bool
    Box<CoordinateType>::isInside(const Vector2<CoordinateType>& point) const noexcept {
      return getLeftBound() <= point.x() &&
             getRightBound() >= point.x() &&
             getBottomBound() <= point.y() &&
             getTopBound() >= point.y();
    }

    template <typename CoordinateType>
    inline
    Vector2<CoordinateType>
    Box<CoordinateType>::getNearestPoint(const Vector2<CoordinateType>& point) const noexcept {
      return Vector2<CoordinateType>(
        getLeftBound() > point.x() ? getLeftBound() : (getRightBound() < point.x() ? getRightBound() : point.x()),
        getBottomBound() > point.y() ? getBottomBound() : (getTopBound() < point.y() ? getTopBound() : point.y())
      );
    }

    template <typename CoordinateType>
    inline
    SDL_Rect
    Box<CoordinateType>::toSDLRect() const noexcept {
      return SDL_Rect{
        static_cast<int>(getLeftBound()),
        static_cast<int>(getBottomBound()),
        static_cast<int>(w()),
        static_cast<int>(h())
      };
    }

  }
}

#endif    /* BOX_HXX_INCLUDED */
