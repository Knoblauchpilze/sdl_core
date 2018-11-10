#ifndef    SDLLAYOUT_HH
# define   SDLLAYOUT_HH

# include <memory>
# include <vector>
# include "Box.hh"

namespace sdl {
  namespace core {

    using Boxf = sdl::utils::Box<float>;

    class SdlWidget;

    // Slave class of SdlWidget.
    class SdlLayout {
      public:

        SdlLayout(SdlWidget* widget = nullptr);

        virtual ~SdlLayout();

        void
        update(const Boxf& area);

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
        makeDirty() noexcept;

      private:

        SdlWidget*
        getContainerOrNull(SdlWidget* item, int* index = nullptr) const;

      protected:

        SdlWidget* m_widget;
        std::vector<SdlWidget*> m_items;
        bool m_dirty;

    };

    using SdlLayoutShPtr = std::shared_ptr<SdlLayout>;
  }
}

# include "SdlLayout.hxx"

#endif    /* SDLLAYOUT_HH */
