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
        update();

        virtual int
        addItem(std::shared_ptr<SdlWidget> item);

        virtual int
        addItem(std::shared_ptr<SdlWidget> item,
                const unsigned& x,
                const unsigned& y,
                const unsigned& w,
                const unsigned& h);

        virtual void
        removeItem(std::shared_ptr<SdlWidget> item);

      protected:

        virtual void
        updatePrivate(const Boxf& window) = 0;

      private:

        std::shared_ptr<SdlWidget>
        getContainerOrNull(std::shared_ptr<SdlWidget> item, int* index = nullptr) const;

      protected:

        SdlWidget* m_widget;
        std::vector<std::shared_ptr<SdlWidget>> m_items;

    };

    using SdlLayoutShPtr = std::shared_ptr<SdlLayout>;
  }
}

# include "SdlLayout.hxx"

#endif    /* SDLLAYOUT_HH */
