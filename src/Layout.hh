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

    class Layout: public LayoutItem {
      public:

        /**
         * @brief - Describes the kind of nesting associated to a
         *          layout item. Nesting allows to compose several
         *          layers of layout which in turn allows layouts
         *          managed by other layouts.
         *          Basically a layout item can be a `Root` item
         *          (which means that it is at the top level of
         *          its layout hierarchy) or `Nested` (which tells
         *          that some other layout controls the size and
         *          aspect of this item).
         */
        enum class Nesting {
          Root,  //<!- Indicates that this item is at the top of
                 //<!  its layout hierarchy.
          Deep   //<!- Indicates that this item is nested under
                 //<!  another layer of layout.
        };

        /**
         * @brief - Describes the format of the bounding box expected
         *          by this layout and more precisely by the method
         *          `assignRenderingAreas`.
         *          Depending on whether the boxes are produced in a
         *          window or a widget, their format can vary a bit and
         *          in order to keep the interface of the layout kinda
         *          consistent we allow upon defining a layout to tell
         *          which format is used.
         */
         enum class BoxesFormat {
           Window,  //<!- Boxes are provided to the `assignRenderingAreas`
                    //<!  method in window format.
           Engine   //<!- Boxes are provided to the `assignRenderingAreas`
                    //<!  method in engine format.
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
         * @brief - Used to determine whether this layout is nested under another layout
         *          or if it is at the top of its layout hierarchy.
         * @return - `true` if this layout is NOT at the top of its hierarchy, `false`
         *           otherwise.
         */
        bool
        isNested() const noexcept;

        /**
         * @brief - Sets this layout's nesting status to be equal to the value in argument
         *          Allows to define whether a layout is at the top level of its layout
         *          hierarchy.
         * @param nesting - a textual description of the nesting status of this layout.
         */
        void
        setNesting(const Nesting& nesting);

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

        /**
         * @brief - Builds a layout object with the specified name, parent widget
         *          and margin.
         *          In addition the user can also specify the format into which the
         *          bounding boxes will be provided to the `assignRenderingAreas`
         *          for this widget. Indeed as the computation of the bounding boxes
         *          to assign to managed items is done in the pure virtual method
         *          `computeGeometry` we have no way of determining it beforehand.
         *          This feature is useful for layouts attached to a window and not
         *          a widget to keep a consistent interface.
         * @param name - the name to assign to this layout.
         * @param widget - a pointer to the widget which is managed by this layout.
         *                 This widget can still be set later using the `SdlWidget`
         *                 method `setLayout`.
         * @param margin - a float value indicating how much margin should be set
         *                 around the managed item. Note that a single value is
         *                 needed which means that both horizontal and vertical
         *                 margins will always be the same.
         * @param type - the type of this layout: this value is transmitted to the
         *               base `LayoutItem` class and indicates whether 
         */
        Layout(const std::string& name,
               SdlWidget* widget = nullptr,
               const float& margin = 0.0f,
               const BoxesFormat& format = BoxesFormat::Engine);

        /**
         * @brief - Reimplementation of the base `LayoutItem` method in order to
         *          provide a rebuild of the layout upon receiving a geometry update
         *          event. This will trigger a computation of the size of all the
         *          managed events. Usually the input `window` corresponds to the size
         *          of the widget managed by this layout.
         * @param window - a box representing the available size for this layout.
         */
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
         * @brief - Used to determine whether the bounding boxes to assign to the
         *          items managed by this layout need to be converted into an
         *          engine format before being assigned.
         *          This is particularly useful in order to allow top level layout
         *          (such as ones which are associated directly to a window) to
         *          easily indicate to their children that the bounding boxes that
         *          is computed by the layout do not need to be converted.
         *          In order to make the interface to the `LayoutItem` consistent,
         *          the conversion is handled upon calling the `assignRenderingAreas`
         *          method in this object. This is were the status returned by this
         *          method will probably be most useful.
         * @return - `true` if the bounding boxes to assign to children items need
         *           to be converted into engine format first and `false` otherwise.
         *           Note that the `true` return value actually also indicates that
         *           the internal boxes format is set to `Engine`.
         */
        bool
        needsConvert() const noexcept;

        /**
         * @brief - Used to assign a new boxes format to this layout. The new format
         *          overrides the old one and is applied right away.
         * @param format - the new bounding boxes format sent to the internal method
         *                 `assignRenderingAreas`.
         */
        void
        setBoxesFormat(const BoxesFormat& format);

        /**
         * @brief - Redefintion of the base `EngineObject` method which allows to
         *          react to children widgets gaining focus by propagating the
         *          information to siblings widget for top level items.
         * @param e - the gain focus event to propagate to siblings widget. We
         *            assume that the emitter is one of the children of this layout.
         * @return - true if the event was recognized, false otherwise.
         */
        bool
        gainFocusEvent(const engine::FocusEvent& e) override;

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
        canBeUsedTo(const WidgetInfo& info,
                    const utils::Boxf& box,
                    const SizePolicy& action) const;

      private:

        /// Used to give access to `SdlWidget` to protected method on this class.
        friend class SdlWidget;

        /**
         * @brief - Convenience define which represents a list of pointer to some
         *          `LayoutItem` object: useful to refer to the internal array of
         *          managed objects by a layout.
         */
        using Items = std::vector<LayoutItem*>;

        /**
         * @brief - Contains the list of all the managed items by this layout. Items
         *          are inserted in a chronological order into this array.
         */
        Items        m_items;

        /**
         * @brief - Margin to use when computing the size available for children widgets. Basically
         *          when given an available space to allocate between widgets, we subtract first the
         *          value provided by the margin in order to allow some nice outline.
         *          This value can be set to `0` if no margin is desirable.
         */
        utils::Sizef m_margin;

        /**
         * @brief - Used to determine whether the boxes provided by this item using the method called
         *          `assignRenderingAreas` should be converted before assigning them to the various
         *          items or not. This is controlled by the enumeration value defined for convenience.
         */
        BoxesFormat  m_boxesFormat;

        /**
         * @brief - Used to determine whether this layout is nested into another layout, meaning that
         *          the bounding boxes provided to the `assignRenderingAreas` need to be offseted with
         *          the available space position. This typically indicates that the item is NOT at the
         *          top of its layout hiearchy and is strongly tied to the `m_boxesFormat` attribute.
         */
        Nesting      m_nesting;
    };

    using LayoutShPtr = std::shared_ptr<Layout>;
  }
}

# include "Layout.hxx"

#endif    /* LAYOUT_HH */
