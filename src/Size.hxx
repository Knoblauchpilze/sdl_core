#ifndef    SIZE_HXX
# define   SIZE_HXX

# include <limits>
# include "Size.hh"

namespace sdl {
  namespace utils {

    template <typename DimsType>
    inline
    Size<DimsType>::Size(const DimsType& width,
                         const DimsType& height):
      m_w(width),
      m_h(height)
    {}

    template <typename DimsType>
    inline
    bool
    Size<DimsType>::operator==(const Size& rhs) const noexcept {
      return m_w == rhs.m_w && m_h == rhs.m_h;
    }

    template <typename DimsType>
    inline
    bool
    Size<DimsType>::operator!=(const Size& rhs) const noexcept {
      return !operator==(rhs);
    }

    template <typename DimsType>
    inline
    Size<DimsType>
    Size<DimsType>::operator+(const Size& rhs) const noexcept {
      return Size(m_w + rhs.m_w, m_h + rhs.m_h);
    }

    template <typename DimsType>
    DimsType&
    Size<DimsType>::w() noexcept {
      return m_w;
    }

    template <typename DimsType>
    inline
    const DimsType&
    Size<DimsType>::w() const noexcept {
      return m_w;
    }

    template <typename DimsType>
    DimsType&
    Size<DimsType>::h() noexcept {
      return m_h;
    }

    template <typename DimsType>
    inline
    const DimsType&
    Size<DimsType>::h() const noexcept {
      return m_h;
    }

    template <typename DimsType>
    inline
    void
    Size<DimsType>::setWidth(const DimsType& width) noexcept {
      m_w = width;
    }

    template <typename DimsType>
    inline
    void
    Size<DimsType>::setHeight(const DimsType& height) noexcept {
      m_h = height;
    }

    template <typename DimsType>
    inline
    bool
    Size<DimsType>::isEmpty() const noexcept {
      return m_w == DimsType() || m_h == DimsType();
    }

    template <typename DimsType>
    inline
    bool
    Size<DimsType>::isNull() const noexcept {
      return m_w == DimsType() && m_h == DimsType();
    }

    template <typename DimsType>
    inline
    bool
    Size<DimsType>::isValid() const noexcept {
      return !isEmpty();
    }

    template <typename DimsType>
    inline
    void
    Size<DimsType>::transpose() noexcept {
      const DimsType save = m_w;
      m_w = m_h;
      m_h = save;
    }

    template <typename DimsType>
    inline
    Size<DimsType>
    Size<DimsType>::max() noexcept {
      return Size(
        std::numeric_limits<DimsType>::max(),
        std::numeric_limits<DimsType>::max()
      );
    }

  }
}

#endif    /* SIZE_HXX */
