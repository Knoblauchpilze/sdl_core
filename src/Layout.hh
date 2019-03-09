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

        void
        assignRenderingAreas(const std::vector<sdl::utils::Boxf>& boxes);

        sdl::utils::Sizef
        computeSizeOfWidgets(const Direction& direction,
                             const std::vector<sdl::utils::Boxf>& boxes) const;

        sdl::utils::Sizef
        computeSizeFromPolicy(const sdl::utils::Sizef& desiredSize,
                              const sdl::utils::Boxf& currentSize,
                              const WidgetInfo& info) const;

        sdl::core::SizePolicy
        shrinkOrGrow(const sdl::utils::Sizef& desiredSize,
                     const sdl::utils::Sizef& achievedSize) const;

        bool
        canBeUsedTo(const WidgetInfo& info,
                    const sdl::utils::Boxf& box,
                    const SizePolicy& action) const;

        bool
        canBeUsedTo(const WidgetInfo& info,
                    const sdl::utils::Boxf& box,
                    const SizePolicy::Policy& action,
                    const Direction& direction) const;

        sdl::utils::Sizef
        computeSpaceAdjustmentNeeded(const sdl::utils::Sizef& achieved,
                                     const sdl::utils::Sizef& target) const;

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
