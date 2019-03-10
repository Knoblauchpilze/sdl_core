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
        updatePrivate(const utils::maths::Boxf& window) = 0;

        void
        invalidate() noexcept;

      protected:

        struct WidgetInfo {
          sdl::core::SizePolicy policy;
          utils::maths::Sizef min;
          utils::maths::Sizef hint;
          utils::maths::Sizef max;
          utils::maths::Boxf area;
        };

        std::vector<WidgetInfo>
        computeWidgetsInfo() const noexcept;

        utils::maths::Sizef
        computeIncompressibleSize(const Direction& direction,
                                  const std::vector<WidgetInfo>& widgets) const;

        void
        assignRenderingAreas(const std::vector<utils::maths::Boxf>& boxes);

        utils::maths::Sizef
        computeSizeOfWidgets(const Direction& direction,
                             const std::vector<utils::maths::Boxf>& boxes) const;

        utils::maths::Sizef
        computeSizeFromPolicy(const utils::maths::Sizef& desiredSize,
                              const utils::maths::Boxf& currentSize,
                              const WidgetInfo& info) const;

        sdl::core::SizePolicy
        shrinkOrGrow(const utils::maths::Sizef& desiredSize,
                     const utils::maths::Sizef& achievedSize,
                     const float& tolerance) const;

        std::pair<bool, bool>
        canBeUsedTo(const std::string& name,
                    const WidgetInfo& info,
                    const utils::maths::Boxf& box,
                    const SizePolicy& action) const;

        utils::maths::Sizef
        computeSpaceAdjustmentNeeded(const utils::maths::Sizef& achieved,
                                     const utils::maths::Sizef& target) const;

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
