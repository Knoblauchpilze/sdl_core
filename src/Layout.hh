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

        virtual void
        update();

        virtual int
        addItem(SdlWidget* item);

        virtual int
        addItem(SdlWidget* item,
                const unsigned& x,
                const unsigned& y,
                const unsigned& w,
                const unsigned& h);

        virtual int
        removeItem(SdlWidget* item);

        int
        getItemsCount() const noexcept;

        bool
        empty() const noexcept;

        const utils::Sizef&
        getMargin() const noexcept;

      protected:

        Layout(SdlWidget* widget = nullptr,
               const float& margin = 0.0f,
               const bool allowLog = false,
               const std::string& name = std::string("Layout"),
               const bool rootLayout = false);

        virtual void
        updatePrivate(const utils::Boxf& window) = 0;

        /**
         * @brief - Used internally to determine whether this layout needs to be recompued
         *          or if the cached data still applies.
         * @return - true if the layout needs to be recomputed and false otherwise.
         */
        bool
        isDirty() const noexcept;

        /**
         * @brief - Used internally to mark the layout as recomputed. This will prevent later
         *          calls to `update` to trigger a full recompuation of the layout. A new rebuild
         *          will only occur when a call to `invalidate` is done.
         *          Note that this method should be called with care and only after effectively
         *          rebuilding the layout, otherwise some rebuild events might get ignored.
         */
        void
        recomputed();

        bool
        isRootLayout() const noexcept;

        void
        setRootLayout(const bool root) noexcept;

        int
        getIndexOf(SdlWidget* item) const noexcept;

        int
        getIndexOf(const std::string& name) const noexcept;

        SdlWidget*
        getWidgetAt(const int& item);

        const SdlWidget*
        getWidgetAt(const int& item) const;

        bool
        isValidIndex(const int& id) const noexcept;

        virtual void
        removeItemFromIndex(int item);

        /**
         * @brief - Used internally to mark the layout as dirty and to trigger a recomputation on
         *          the next call to `update`. THis method should only be called when the area
         *          allocated to the layout has changed or in case of an addition/removal of a widget
         *          in/from the layout. Inheriting classes are encouraged to override this method in
         *          case some additional information should be performed upon invalidating the layout.
         *          This can include for example rebuilding specialized tables used to retain more
         *          information about the position of the widgets inserted in the layout and such.
         */
        virtual void
        invalidate() noexcept;

        virtual utils::Sizef
        computeAvailableSize(const utils::Boxf& totalArea) const noexcept;

        /**
         * @brief - Assigned the rendering areas defined in the input `boxes` vector based on their
         *          position in the vector to the corresponding widget in the internal `m_items` array.
         *          The areas are translated from a top left representation into a centered
         *          representation using the provided `window` parameter which describes the absolute
         *          position of the coordinate frame each box is related to.
         *          If the coordinates should not be converted, one can set the `root layout` property
         *          for this layout either at construction or using the `setRootLayout` method.
         *          In this case the `window` argument will be ignored and the bounding boxes will be
         *          assigned as is.
         * @param boxes - an array of boxes supposedly of the same length as `m_items` array where each
         *                box will be assigned to the corresponding item in the internal array.
         * @param window - the absolute position of the box containing the input boxes. This allows to
         *                 perform a conversion of the input boxes so that they can be correctly drawn
         *                 by the rendering engine.
         */
        void
        assignRenderingAreas(const std::vector<utils::Boxf>& boxes,
                             const utils::Boxf& window);

        void
        assignVisibilityStatus(const std::vector<bool>& visible);

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
        allocateFairly(const float& space,
                       const unsigned& count) const noexcept;

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

        friend class SdlWidget;

        SdlWidget* m_widget;
        std::vector<SdlWidget*> m_items;
        bool m_dirty;
        utils::Sizef m_margin;

        bool m_rootLayout;
    };

    using LayoutShPtr = std::shared_ptr<Layout>;
  }
}

# include "Layout.hxx"

#endif    /* LAYOUT_HH */
