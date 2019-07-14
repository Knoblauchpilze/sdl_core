#ifndef    LAYOUT_HH
# define   LAYOUT_HH

# include <memory>
# include <vector>
# include <maths_utils/Box.hh>
# include <maths_utils/Size.hh>

# include "LayoutItem.hh"
# include "SizePolicy.hh"

namespace sdl {
  namespace core {

    class SdlWidget;

    // Slave class of SdlWidget.
    class Layout: public LayoutItem {
      public:

        enum class Direction {
          Horizontal,
          Vertical
        };

      public:

        virtual ~Layout();

        /**
         * @brief - Adds the specified `item` to this layout and returns the index of the item
         *          in the layout. This method calls internally the `addItem` method with the
         *          logical id equal to the current items count.
         *          The index returned by this function corresponds to a physical index.
         *          Note that if the `item` is not considered valid a negative value is returned.
         * @param itme - the item to add to this layout.
         * @return - a positive value corresponding to the index of the item in the layout if it
         *           could successfully be added or a negative value (usually `-1`) if something
         *           went wrong.
         */
        virtual int
        addItem(LayoutItem* item);

        /**
         * @brief - Adds the specified `item` to this layout and returns the index of the item
         *          in the layout.
         *          Note that this function is not specialized in this class and thus is similar
         *          to calling the `addItem` method.
         *          The method calls the `onIndexAdded` right after inserting the `item` in the
         *          layout. The input index is assumed to be a logical index.
         * @param item - the item to add to this layout.
         * @param index - the position at which the item should be added in this layout.
         */
        virtual void
        addItem(LayoutItem* item,
                const int& index);

        /**
         * @brief - Adds the specified `item` to this layout and returns the index of the item
         *          in the layout. The user can specify the position of the item using a grid
         *          and dimensions.
         *          Note that this function is not specialized in this class and thus is similar
         *          to calling the `addItem` method.
         * @param item - the item to add to this layout.
         * @param x - the abscissa of the position to which the item should be added.
         * @param y - the ordinate of the position to which the item should be added.
         * @param w - the width of the item in this layout in terms of cells.
         * @param h - the height of the item in this layout in temrs of cells.
         */
        virtual void
        addItem(LayoutItem* item,
                const unsigned& x,
                const unsigned& y,
                const unsigned& w,
                const unsigned& h);

        /**
         * @brief - Performs the deletion of the onput `item` from the layout if it exists. The
         *          return value indicates the id at which this item was placed. A negative value
         *          is returned if the item does not exist in this layout or is not valid.
         *          Note that this method internally calls `removeItemFromIndex` with a logical
         *          index retrieved by calling the `getLogicalIDFromPhysicalID` method.
         * @param item - the index which should be removed from the layout.
         * @return - the logical index at which the item was removed, or a negative value if no
         *           such item exists in this layout.
         */
        virtual int
        removeItem(LayoutItem* item);

        /**
         * @brief - Removes the item at the specified `index` in the layout. Note that the `removeItem`
         *          method with a `LayoutItem` as argument uses this method internally to perform its
         *          deletion.
         *          Inheriting classes are encouraged to specialize this method in order to provide
         *          custom behavior on what exactly the item at `index` means.
         *          Note that the input `index` is assumed to be a logical index and is thus transformed
         *          using the `getPhysicalIDFromLogicalID` method before using it.
         *          Triggers a call to `onIndexRemoved` right before triggering the `GeometryUpdate`
         *          event creation.
         * @param index - the index of the item which should be removed, whatever that means.
         */
        virtual void
        removeItemFromIndex(const int index);

        int
        getItemsCount() const noexcept;

        bool
        empty() const noexcept;

        const utils::Sizef&
        getMargin() const noexcept;

        /**
         * @brief - Used in case this layout is a virtual layout to trigger a
         *          recomputation of this object. Note that most users should
         *          never use this function and rather use the standard way
         *          of adding item and letting the events system do its job,
         *          but in the case of a layout which is used by another layout
         *          for example, this method can be triggered when the parent
         *          layout needs to take advantage of the virtual layout to
         *          perform its processing.
         *          Note that this method will do nothing if the layout is not
         *          virtual.
         * @param window - the available space for the layout to be computed.
         */
        void
        update(const utils::Boxf& window);

        /**
         * @brief - Reimplementation of the `EngineObject` class method so
         *          that we also assign the events queue to the children
         *          item if any.
         * @param queue - the events queue to assign to this layout.
         */
        void
        setEventsQueue(engine::EventsQueue* queue) noexcept override;

      protected:

        Layout(const std::string& name,
               SdlWidget* widget = nullptr,
               const float& margin = 0.0f);

        void
        updatePrivate(const utils::Boxf& window) override;

        /**
         * @brief - Interface method called by the `updatePrivate` method which is
         *          triggered after the size of this layout has been updated and
         *          some checkes have been performed to determine whether this
         *          layout is empty or not.
         *          Note that inheriting classes need to redefine this method in
         *          order to be concrete implementation of a layout object.
         * @param window - the box describing the available space for this layout.
         */
        virtual void
        computeGeometry(const utils::Boxf& window) = 0;

        /**
         * @brief - Redefinition of the base `EngineObject` class. This method
         *          will allow to transmit the paint event to children which have
         *          an overlap with one of the area described in the event.
         *          This mechanism is used to transmit events from siblings items
         *          to other.
         * @param e - the paint event to process.
         * @return - true if the event was recognized, false otherwise.
         */
        bool
        repaintEvent(const engine::PaintEvent& e) override;

        /**
         * @brief - Try to retrieve the index of the item specified as argument.
         *          The returned index corresponds to the physical order of this
         *          item in the internal data.
         *          Note that if the item is not valid or does not exist in the
         *          layout, a negative value is returned (usually `-1`).
         *          We use pointer equality to determine whether items are equal.
         * @param item - the item for which the index should be returned.
         * @return - the index of the item in this layout or a negative value if
         *           no such item exists in the layout.
         */
        int
        getIndexOf(LayoutItem* item) const noexcept;

        /**
         * @brief - Try to retrieve the index of the item specified as argument.
         *          The returned index corresponds to the physical order of this
         *          item in the internal data.
         *          Note that if the item is not valid or does not exist in the
         *          layout, a negative value is returned (usually `-1`).
         *          Also note that the first item with a name matching the input
         *          string is returned, which does not mean it is the only one
         *          inserted in this widget.
         * @param item - the item for which the index should be returned.
         * @return - the index of the item in this layout or a negative value if
         *           no such item exists in the layout.
         */
        int
        getIndexOf(const std::string& name) const noexcept;

        /**
         * @brief - Used to retrieve the logical id associated to the input id considered
         *          as a physical id. At this level this method basically returns the input
         *          index but inheriting classes are encouraged to reimplement this in
         *          order to specialize the behavior with potential other associations made
         *          by them on indices.
         *          Note that if no such id can be found, a negative value is returned.
         * @param physID - the physical id for which the logical id should be returned.
         * @return - an logical index which corresponds to the input physical id or a
         *           negative value if no such index exists in the layout.
         */
        virtual int
        getLogicalIDFromPhysicalID(const int physID) const noexcept;

        /**
         * @brief - Inverse method to `getLogicalIDFromPhysicalID`. Calling both methods
         *          successively should return the input index.
         *          Note that if no such id can be found, a negative value is returned.
         * @param logicID - the logical id for which the physical id should be returned.
         * @return - an physical index which corresponds to the input logical id or a
         *           negative value if no such index exists in the layout.
         */
        virtual int
        getPhysicalIDFromLogicalID(const int logicID) const noexcept;

        /**
         * @brief - Called right after removing the logical index corresponding to the physical
         *          index in argument. This allows inheriting classes to react to the removal
         *          of an item and update their own internal data. The user has the possibility
         *          through the return value to trigger a recompute of the layout. In most
         *          layouts removing an item should trigger a rebuild of the layout (as the
         *          deleted item took up some place which can now be redistributed to other
         *          elements) but this is not the case for all layouts.
         *          Note that as the removal has already taken place, calling `isValidIndex` on
         *          the `physID` might sometims fail.
         * @param logicID - the logical id which has just been removed.
         * @param physID - the physical id associated to the logical id removed.
         * @return - true if the layout should be invalidated, false otherwise. If the return
         *           value is false no `GeometryUpdate` event will be posted for this layout.
         */
        virtual bool
        onIndexRemoved(const int logicID,
                       const int physID);

        LayoutItem*
        getItemAt(const int& item);

        const LayoutItem*
        getItemAt(const int& item) const;

        bool
        isValidIndex(const int& id) const noexcept;

        virtual utils::Sizef
        computeAvailableSize(const utils::Boxf& totalArea) const noexcept;

        /**
         * @brief - Assigned the rendering areas defined in the input `boxes` vector based on their
         *          position in the vector to the corresponding item in the internal `m_items` array.
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
          bool visible;
        };

        std::vector<WidgetInfo>
        computeItemsInfo() const noexcept;

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

        using Items = std::vector<LayoutItem*>;

        Items m_items;
        utils::Sizef m_margin;
    };

    using LayoutShPtr = std::shared_ptr<Layout>;
  }
}

# include "Layout.hxx"

#endif    /* LAYOUT_HH */
