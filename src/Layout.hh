#ifndef    LAYOUT_HH
# define   LAYOUT_HH

# include <memory>
# include <vector>
# include "Box.hh"

namespace sdl {
  namespace core {

    using Boxf = sdl::utils::Box<float>;

    class SdlWidget;

    // Slave class of SdlWidget.
    class Layout {
      public:

        Layout(SdlWidget* widget = nullptr);

        virtual ~Layout();

        void
        update();

        virtual int
        addItem(SdlWidget* item);

        virtual int
        addItem(SdlWidget* item,
                const unsigned& x,
                const unsigned& y,
                const unsigned& w,
                const unsigned& h);

        virtual void
        removeItem(SdlWidget* item);

        unsigned
        getItemsCount() const noexcept;

      protected:

        virtual void
        updatePrivate(const Boxf& window) = 0;

        void
        invalidate() noexcept;

      private:

        SdlWidget*
        getContainerOrNull(SdlWidget* item, int* index = nullptr) const;

      protected:

        SdlWidget* m_widget;
        std::vector<SdlWidget*> m_items;
        bool m_dirty;

    };

    using LayoutShPtr = std::shared_ptr<Layout>;
  }
}

# include "Layout.hxx"

#endif    /* LAYOUT_HH */
