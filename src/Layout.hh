#ifndef    LAYOUT_HH
# define   LAYOUT_HH

# include <memory>
# include <vector>
# include "Box.hh"
# include "Size.hh"
# include "SizePolicy.hh"

namespace sdl {
  namespace core {

    class SdlWidget;

    // Slave class of SdlWidget.
    class Layout {
      public:

        enum class Direction {
          Horizontal,
          Vertical
        };

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
        updatePrivate(const sdl::utils::Boxf& window) = 0;

        void
        invalidate() noexcept;

      protected:

        struct WidgetInfo {
          sdl::core::SizePolicy policy;
          sdl::utils::Sizef min;
          sdl::utils::Sizef hint;
          sdl::utils::Sizef max;
          sdl::utils::Boxf area;
        };

        std::vector<WidgetInfo>
        computeWidgetsInfo() const noexcept;

        sdl::utils::Sizef
        computeIncompressibleSize(const Direction& direction,
                                  const std::vector<WidgetInfo>& widgets) const;

        std::pair<bool, bool>
        canExpand(const WidgetInfo& info,
                  const sdl::utils::Sizef& size) const;

      private:

        SdlWidget*
        getContainerOrNull(SdlWidget* item, int* index = nullptr) const;

        bool
        canExpand(const WidgetInfo& info,
                  const Direction& direction,
                  const float& desiredSize) const;

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
