#ifndef    BOX_HH
# define   BOX_HH

# include <SDL2/SDL.h>
# include "Vector2.hh"

namespace sdl {
  namespace utils {

    template <typename CoordinateType>
    class Box {
      public:

        /*
         * @brief - Describes a box centered at coordinate (x, y) with the specified dimensions (width, height)
         * @param x - the x coordinate of the center.
         * @param y - the y coordinate of the center.
         * @param width - the width of the box.
         * @param height - the height of the box.
         */
        explicit
        Box(const CoordinateType& x = CoordinateType(),
            const CoordinateType& y = CoordinateType(),
            const CoordinateType& width = CoordinateType(),
            const CoordinateType& height = CoordinateType()) noexcept;

        CoordinateType&
        x() noexcept;

        const CoordinateType&
        x() const noexcept;

        CoordinateType&
        y() noexcept;

        const CoordinateType&
        y() const noexcept;

        CoordinateType&
        w() noexcept;

        const CoordinateType&
        w() const noexcept;

        CoordinateType&
        h() noexcept;

        const CoordinateType&
        h() const noexcept;

        CoordinateType
        getLeftBound() const noexcept;

        CoordinateType
        getRightBound() const noexcept;

        CoordinateType
        getTopBound() const noexcept;

        CoordinateType
        getBottomBound() const noexcept;

        Vector2<CoordinateType>
        getCenter() const noexcept;

        Vector2<CoordinateType>
        getTopLeftCorner() const noexcept;

        Vector2<CoordinateType>
        getTopRightCorner() const noexcept;

        Vector2<CoordinateType>
        getBottomRightCorner() const noexcept;

        Vector2<CoordinateType>
        getBottomLeftCorner() const noexcept;

        CoordinateType
        getSurface() const noexcept;

        /*
         * @brief - Checks whether the input box is contained inside this box.
         * @param other - the other box to check for inclusion.
         * @return - true if the other box is contained inside this box, false otherwise.
         */
        bool
        contains(const Box<CoordinateType>& other) const noexcept;

        /*
         * @brief - Checks whether the input box intersects this box.
         * @param other - the other box to check for inclusion.
         * @return - true if the other box is contained inside this box, false otherwise.
         */
        bool
        intersects(const Box<CoordinateType>& other) const noexcept;

        /*
         * @brief - Checks whether the input box contains this box.
         * @param other - the box into which this box should be contained.
         * @return - true if the other box contains this box, false otherwise.
         */
        bool
        isInside(const Box<CoordinateType>& other) const noexcept;

        /*
         * @brief - Checks whether the input point is inside this box.
         * @param point - the input point to check for inclusion in this box.
         * @return - true if the point lies inside this box, false otherwise.
         */
        bool
        isInside(const Vector2<CoordinateType>& point) const noexcept;

        /*
         * @brief - Retrieves the nearest point to the input 'point' belonging to this box.
         * @param point - the point for which a box point should be retrieved.
         * @return - a vector representing the closest point belonging to this box relatively to the input point.
         */
        Vector2<CoordinateType>
        getNearestPoint(const Vector2<CoordinateType>& point) const noexcept;

        /*
         * @brief - Builds a top left SDL_Rect object from this box.
         * @return - a sdl rect object based on the input type of this object.
         */
        SDL_Rect
        toSDLRect() const noexcept;

      private:

        CoordinateType m_x;
        CoordinateType m_y;
        CoordinateType m_w;
        CoordinateType m_h;
    };

  }
}

# include "Box.hxx"

#endif    /* BOX_HH */
