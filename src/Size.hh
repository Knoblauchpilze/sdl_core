#ifndef    SIZE_HH
# define   SIZE_HH

namespace sdl {
  namespace utils {

    template <typename DimsType>
    class Size {
      public:

        Size(const DimsType& width = DimsType(),
             const DimsType& height = DimsType());

        ~Size() = default;

        const DimsType&
        w() const noexcept;

        const DimsType&
        h() const noexcept;

        void
        setWidth(const DimsType& width) noexcept;

        void
        setHeight(const DimsType& height) noexcept;

        bool
        isEmpty() const noexcept;

        bool
        isNull() const noexcept;

        bool
        isValid() const noexcept;

        void
        transpose() noexcept;

        static
        Size
        max() noexcept;

      public:

      private:

        DimsType m_w;
        DimsType m_h;

    };

  }
}

# include "Size.hxx"

#endif    /* SIZE_HH */
