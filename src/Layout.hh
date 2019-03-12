#ifndef    LAYOUT_HH
# define   LAYOUT_HH

# include <memory>
# include <vector>
# include <maths_utils/Box.hh>
# include <maths_utils/Size.hh>

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
        updatePrivate(const utils::Boxf& window) = 0;

        void
        invalidate() noexcept;

      protected:

        struct WidgetInfo {
          sdl::core::SizePolicy policy;
          utils::Sizef min;
          utils::Sizef hint;
          utils::Sizef max;
          utils::Boxf area;
        };

        std::vector<WidgetInfo>
        computeWidgetsInfo() const noexcept;

        utils::Sizef
        computeIncompressibleSize(const Direction& direction,
                                  const std::vector<WidgetInfo>& widgets) const;

        void
        assignRenderingAreas(const std::vector<utils::Boxf>& boxes);

        utils::Sizef
        computeSizeOfWidgets(const Direction& direction,
                             const std::vector<utils::Boxf>& boxes) const;

        utils::Sizef
        computeSizeFromPolicy(const utils::Sizef& desiredSize,
                              const utils::Boxf& currentSize,
                              const WidgetInfo& info) const;

        sdl::core::SizePolicy
        shrinkOrGrow(const utils::Sizef& desiredSize,
                     const utils::Sizef& achievedSize,
                     const float& tolerance) const;

        std::pair<bool, bool>
        canBeUsedTo(const std::string& name,
                    const WidgetInfo& info,
                    const utils::Boxf& box,
                    const SizePolicy& action) const;

        utils::Sizef
        computeSpaceAdjustmentNeeded(const utils::Sizef& achieved,
                                     const utils::Sizef& target) const;

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
