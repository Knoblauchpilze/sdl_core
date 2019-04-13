#ifndef    LAYOUT_HH
# define   LAYOUT_HH

# include <memory>
# include <vector>
# include <core_utils/CoreObject.hh>
# include <maths_utils/Box.hh>
# include <maths_utils/Size.hh>

# include "SizePolicy.hh"

namespace sdl {
  namespace core {

    class SdlWidget;

    // Slave class of SdlWidget.
    class Layout: public utils::CoreObject {
      public:

        enum class Direction {
          Horizontal,
          Vertical
        };

      public:

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

        const utils::Sizef&
        getMargin() const noexcept;

      protected:

        Layout(SdlWidget* widget = nullptr,
               const float& margin = 0.0f,
               const std::string& name = std::string("Layout"));

        virtual void
        updatePrivate(const utils::Boxf& window) = 0;

        void
        invalidate() noexcept;

        virtual utils::Sizef
        computeAvailableSize(const utils::Boxf& totalArea) const noexcept;

        void
        assignRenderingAreas(const std::vector<utils::Boxf>& boxes);

        utils::Sizef
        computeSpaceAdjustmentNeeded(const utils::Sizef& achieved,
                                     const utils::Sizef& target) const;

        SizePolicy
        shrinkOrGrow(const utils::Sizef& desiredSize,
                     const utils::Sizef& achievedSize,
                     const float& tolerance) const;

      protected:

        struct WidgetInfo {
          SizePolicy policy;
          utils::Sizef min;
          utils::Sizef hint;
          utils::Sizef max;
          utils::Boxf area;
        };

        std::vector<WidgetInfo>
        computeWidgetsInfo() const noexcept;

        float
        computeWidthFromPolicy(const utils::Boxf& currentSize,
                               const float& delta,
                               const WidgetInfo& info) const;

        float
        computeHeightFromPolicy(const utils::Boxf& currentSize,
                                const float& delta,
                                const WidgetInfo& info) const;

        utils::Sizef
        computeSizeFromPolicy(const utils::Boxf& currentSize,
                              const utils::Sizef& sizeDelta,
                              const WidgetInfo& info) const;

        std::pair<bool, bool>
        canBeUsedTo(const std::string& name,
                    const WidgetInfo& info,
                    const utils::Boxf& box,
                    const SizePolicy& action) const;

      private:

        SdlWidget*
        getContainerOrNull(SdlWidget* item, std::size_t* index = nullptr) const;

      protected:

        SdlWidget* m_widget;
        std::vector<SdlWidget*> m_items;
        bool m_dirty;
        utils::Sizef m_margin;
    };

    using LayoutShPtr = std::shared_ptr<Layout>;
  }
}

# include "Layout.hxx"

#endif    /* LAYOUT_HH */
