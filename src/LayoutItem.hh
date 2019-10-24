#ifndef    LAYOUT_ITEM_HH
# define   LAYOUT_ITEM_HH

# include <mutex>
# include <memory>
# include <maths_utils/Box.hh>
# include <maths_utils/Size.hh>
# include <sdl_engine/EngineObject.hh>
# include <sdl_engine/FocusEvent.hh>
# include <sdl_engine/MouseEvent.hh>
# include <sdl_engine/KeyEvent.hh>
# include <sdl_engine/DropEvent.hh>
# include "SizePolicy.hh"
# include "FocusPolicy.hh"
# include "FocusState.hh"

namespace sdl {
  namespace core {

    class LayoutItem: public engine::EngineObject {
      public:

        /**
         * @brief - Creates a layout item with the specified name and properties.
         *          A layout item can be considered a `root item` if it is not
         *          embedded into another layer of layout. This has an impact on
         *          the final bboxes produced by this item which will not be
         *          converted if the layout is a top level item.
         *          This comes from the fact that the area provided to a top level
         *          layout item is already converted by the underlying api.
         * @param name - the name of the layout item. Used in the `log` calls as the
         *               last identifier in the log line.
         * @param sizeHint - the hint to use for this layout item: default value is
         *                   an invalid hint which indicates that this layout item
         *                   will be assigned the most relevant size by the layout
         *                   engine.
         * @param needsConvert - true if this item is nested into another layout,
         *                       false otherwise. When this value is true we assume
         *                       that all areas given to the layout item, notably
         *                       through events won't have to be converted before
         *                       using them.
         */
        LayoutItem(const std::string& name,
                   const utils::Sizef& sizeHint = utils::Sizef());

        virtual ~LayoutItem();

        utils::Sizef
        getMinSize() const noexcept;

        void
        setMinSize(const utils::Sizef& size) noexcept;

        utils::Sizef
        getSizeHint() const noexcept;

        void
        setSizeHint(const utils::Sizef& hint) noexcept;

        utils::Sizef
        getMaxSize() const noexcept;

        void
        setMaxSize(const utils::Sizef& size) noexcept;

        SizePolicy
        getSizePolicy() const noexcept;

        void
        setSizePolicy(const SizePolicy& policy) noexcept;

        FocusPolicy
        getFocusPolicy() const noexcept;

        void
        setFocusPolicy(const FocusPolicy& policy) noexcept;

        /**
         * @brief - Describes the rendering area associated to this layout item. The rendering
         *          area corresponds to an abstract surface which is allocated to the layout
         *          item to perform its rendering.
         *          This area is expressed relatively to its parent layout if any.
         * @return - a box representing the rendering area for this layout item including the
         *           transformation in the parent layout.
         */
        virtual utils::Boxf
        getRenderingArea() const noexcept;

        /**
         * @brief - Describes the drawing area associated to this layout item. Compared to the
         *          rendering area a drawing area represents the position at which the layout
         *          item should be drawn.
         *          We can describe two abstract coordinate frame one used for rendering in the
         *          parent frame and one used for drawing in the global coordinate frame.
         *          At this step this method is equivalent to `getRenderingArea` but inheriting
         *          classes are encouraged to specialize it when the meaning of `parent layout`
         *          gets more accurate.
         * @return - a box composing the transformation of this layout item including the local
         *           transformation of this layout item in the parent layout but also the parent
         *           transformation itself.
         */
        virtual utils::Boxf
        getDrawingArea() const noexcept;

        /**
         * @brief - Retrieves the current focus status for this item. Returns `true`
         *          of this item has gained the focus and `false` otherwise.
         * @return - `true` if this item gained focus, `false` otherwise.
         */
        bool
        hasFocus() const noexcept;

        /**
         * @brief - Returns the focus state associated to this item as a reference.
         * @return - a reference to the focus state for this item.
         */
        FocusState&
        getFocusState() noexcept;

        /**
         * @brief - Retrievs the z order for this item.
         * @return - the z order for this item.
         */
        int
        getZOrder() const noexcept;

        /**
         * @brief - Returns a string describing the successvie `Z` order of this item
         *          and its parents, until we reach the `stop` item if this value is
         *          not empty. Typically, in the case of the following hierarchy:
         *          `                  `Z` order  `
         *          `  Item 1              1      `
         *          `    +- Item 2         0      `
         *          `          + Item 3    2      `
         *          A call to this method like `getZOrderString` will return "102" while
         *          a call like `getZOrderString(Item 2)` will return "02".
         *          This is particularly useful to sort and compare items which are not
         *          part of the exact same parent.
         *          Note that this method are encouraged to reimplement this method as
         *          this one is exactly similar to the `getZOrder` one, except it returns
         *          a string rather than an integer.
         * @param stop - the item at which the z ordering string should stop, the process
         *               continue until the root item if this value is left empty.
         * @return - a string representing the successive `z` orders of the ancestors of
         *           this item up until the `stop` item starting with the top most
         *           ancestor.
         */
        virtual
        std::string
        getZOrderString(const LayoutItem* stop = nullptr) const noexcept;

        /**
         * @brief - Assigns a new z order for this item. A new `ZOrderChanged` event will
         *          be issued and directed towards this item.
         *          Note that the value of the z order (i.e. the `m_zOrder` value) gets
         *          modified right away in this method, unlike the visible status upon
         *          `Hide` and `Show` event for example.
         *          The event is merely here to notify listeners that this widget has
         *          changed but it is not in the hand of this item to do anything after
         *          being updated.
         * @param order - the new z order for this item.
         */
        void
        setZOrder(const int order);

        /**
         * @brief - Used to determine whether this item has received the keyboard focus.
         * @return - `true` if this item has received keyboard focus, `false` otherwise.
         */
        bool
        hasKeyboardFocus() const noexcept;

        /**
         * @brief - Returns true if this item's size and position is managed by a
         *          layout and false otherwise.
         * @return - true if this layout item is managed by a super-layout and false
         *           if its position and size are freely defined.
         */
        bool
        isManaged() const noexcept;

        /**
         * @brief - Defines a new manager for this item. A manager is usually responsible
         *          for computing the size and position of an item.
         *          Note that calling this method will override any existing manager.
         * @param item - the new manager for this item.
         */
        void
        setManager(LayoutItem* item) noexcept;

        /**
         * @brief - Used to return the visiblity status for this layout item. Note that
         *          this value might not reflect the most accurate representation of the
         *          status of this item. Indeed if some Hide/Show events are registered
         *          in the internal queue they might change the visibility status during
         *          the next processing step.
         * @return - the current visibility status for this layout item.
         */
        bool
        isVisible() const noexcept;

        /**
         * @brief - Used to assign a new visibility status to this layout item. Note that
         *          in order to play well with the events system this method does not actually
         *          set the visible internal status to `visible`. That is immediately calling
         *          the `isVisible` method afterwards will usually not return `visible`. THe
         *          next events processing pass should occur for that to be true.
         *          This method actually internally triggers a Hide/Show event based on the
         *          value of `visible`.
         * @param visible - the new visibility status. Will trigger the creation of the suited
         *                  visibility event.
         */
        virtual void
        setVisible(bool visible) noexcept;

        /**
         * @brief - Calls the `makeGeometryDirty` internal method. Will trigger a recomputation
         *          of the layout whenever the events loop finds it convenient.
         */
        virtual void
        invalidate();

        /**
         * @brief - Reimplementation of the base `EngineObject` method. This allows to first
         *          separate filtering on mouse event, keyboard event, and some other kind of
         *          events.
         *          Inheriting classes are expected to overload the specialized methods rather
         *          than this one in order to provide consistent behavior across all the items
         *          hierarchy.
         * @param watched - the object for which the filter should be applied.
         * @param e - the event to filter.
         * @return - true if the event should be filtered (i.e. not transmitted to the
         *           `watched` object) and false otherwise.
         */
        bool
        filterEvent(engine::EngineObject* watched,
                    engine::EventShPtr e) override;

        /**
         * @brief - Used to retrieve the most relevant item registered in this layout item at
         *          the specified position. If no such item can be found a null value is returned.
         *          The more specialized item is returned which means that it actually returns
         *          the deepest child in the hierarchy which matches the specified position.
         *          Inheriting classes are supposed to implement this in order to match their
         *          internal definition.
         * @param pos - the position expressed in local window frame for which the best item should
         *              be returned.
         * @return - a pointer to the most relevant child spanning the input position or null if no
         *           such child can be found.
         */
        virtual const LayoutItem*
        getItemAt(const utils::Vector2f& pos) const noexcept = 0;

      protected:

        /**
         * @brief - Reimplementation of the base `EngineObject` method. A layout item is
         *          not meant to process window events which will be reflected in the
         *          return value of this method.
         *          We also keep the focus events active when the item is disabled in
         *          order to still allow propagation of events through the item hierarchy.
         * @param type - the event type which should be checked for filtering.
         * @return - `true` if the event type should be kept active when the object becomes
         *           inactive and `false` otherwise.
         */
        bool
        staysActiveWhileDisabled(const engine::Event::Type& type) const noexcept override;

        /**
         * @brief - Reimplementation of the base `EngineObject` method. A layout item is
         *          not meant to process window events which will be reflected in the
         *          return value of this method.
         * @param type - the event type which should be checked for activation.
         * @return - `true` if the event type should be kept inactive when the object becomes
         *           active and `false` otherwise.
         */
        bool
        staysInactiveWhileEnabled(const engine::Event::Type& type) const noexcept override;

        /**
         * @brief - Retrieves the manager for this item. The manager is usually responsible for
         *          providing a size and position for a layout item.
         *          Note that the return value may be nil if the size and position for this item
         *          are freely defined.
         * @return - a pointer for the manager of this item.
         */
        LayoutItem*
        getManager() const noexcept;

        /**
         * @brief - Used to determine whether this layout item can handle the input focus reason.
         *          We basically use the internally defined `FocusPolicy` to compute the return
         *          value.
         * @param reason - the focus reason which should be analyzed for handling.
         * @return - `true` if the focus reason can be handled, `false` otherwise.
         */
        bool
        canHandleFocusReason(const engine::FocusEvent::Reason& reason) const noexcept;

        virtual void
        makeGeometryDirty();

        virtual bool
        hasGeometryChanged() const noexcept;

        virtual void
        geometryRecomputed() noexcept;

        /**
         * @brief - This method is guaranteed to be called upon handling a meaningful `GeometryUpdate`
         *          event. Inheriting classes are encouraged to specialize this method in order to
         *          provide custom behavior when their size is modified.
         *          The input argument describes the new size to be assigned to the item.
         * @param window - a box representing the available size for this layout item.
         */
        virtual void
        updatePrivate(const utils::Boxf& window);

        /**
         * @brief - Provide a base interface for inheriting classes to be able to filter mouse
         *          events without need to cast anything. This method is called by the base
         *          `filterEvent` method when a mouse event is detected.
         *          Inheriting classes are encouraged to specialize this method to provide
         *          custom filtering on mouse events on various conditions.
         * @param watched - the element for which the mouse event should be filtered. If
         *                  this method returns  `false` the event `e` will be transmitted
         *                  to the `watched` object.
         * @param e - the mouse event which should be filtered. If it is not the case this
         *            method returns false.
         * @return - `true` if this event should be filtered and `false` otherwise.
         */
        virtual bool
        filterMouseEvents(const engine::EngineObject* watched,
                          const engine::MouseEventShPtr e) const noexcept;

        /**
         * @brief - Provide a base interface for inheriting classes to be able to filter
         *          keyboard events without need to cast anything. This method is called by
         *          the base `filterEvent` method when a keyboard event is detected.
         *          Inheriting classes are encouraged to specialize this method to provide
         *          custom filtering on keyboard events on various conditions.
         * @param watched - the element for which the keyboard event should be filtered. If
         *                  this method returns  `false` the event `e` will be transmitted
         *                  to the `watched` object.
         * @param e - the keyboard event which should be filtered. If it is not the case this
         *            method returns false.
         * @return - `true` if this event should be filtered and `false` otherwise.
         */
        virtual bool
        filterKeyboardEvents(const engine::EngineObject* watched,
                             const engine::KeyEventShPtr e) const noexcept;

        /**
         * @brief - Provide a base interface for inheriting classes to be able to filter
         *          drag and drop events without need to cast anything. This method is called
         *          by the base `filterEvent` method when a drag and drop event is detected.
         *          Inheriting classes are encouraged to specialize this method to provide
         *          custom filtering on drag and drop events on various conditions.
         * @param watched - the element for which the drag and drop event should be filtered.
         *                  If this method returns  `false` the event `e` will be transmitted
         *                  to the `watched` object.
         * @param e - the drag and drop event which should be filtered. If it is not the case
         *            this method returns false.
         * @return - `true` if this event should be filtered and `false` otherwise.
         */
        virtual bool
        filterDragAndDropEvents(const engine::EngineObject* watched,
                                const engine::DropEventShPtr e) const noexcept;

        bool
        geometryUpdateEvent(const engine::Event& e) override;

        /**
         * @brief - Reimplementation of the base `EngineObject` method. Will perform the deactivation
         *          of this item for almost all kind of events. This allows efficient management of
         *          items which are hidden as they hardly weigh on the events management system.
         *          The `Show` event still needs to be activated of course (otherwise we would not be
         *          able to reactivate the item when needed) along with some specific events (like
         *          focus handling).
         *          Inheriting classes should reimplement this method if custom operations should be
         *          performed upon hiding the object.
         * @param e - the hide event to process.
         * @return - `true` if the event was recognized, `false` otherwise.
         */
        bool
        hideEvent(const engine::Event& e) override;

        /**
         * @brief - Specialization of the base `EngineObject` method in order to handle keyboard focus
         *          update. This kind of event is triggered when a item should receive the keyboard focus
         *          and thus be sent all the keyboard events.
         * @param e - the keyboard focus event to process.
         * @return - `true` if the event was recognized, `false` otherwise.
         */
        bool
        keyboardGrabbedEvent(const engine::Event& e) override;

        /**
         * @brief - Specialization of the base `EngineObject` method in order to handle keyboard focus
         *          update. This kind of event is triggered when a item should lose the keyboard focus
         *          and thus not receive the keyboard events anymore.
         * @param e - the keyboard focus event to process.
         * @return - `true` if the event was recognized, `false` otherwise.
         */
        bool
        keyboardReleasedEvent(const engine::Event& e) override;

        bool
        resizeEvent(engine::ResizeEvent& e) override;

        bool
        showEvent(const engine::Event& e) override;

      private:

        /**
         * @brief - Describes the size policy for this item. The policy is described using
         *          several sizes which roles are described below. In addition to that, the
         *          `m_sizePolicy` allows to determine the behavior of the item when some
         *          extra space is allocated to it or if it should be srhunk for some reason.
         */
        /**
         * @brief - Holds the minimum size which can be assigned to a item while still making
         *          the item usable. Any area smaller than this would make the item useless
         *          as the user would not be able to properly use it.
         */
        utils::Sizef m_minSize;

        /**
         * @brief - Holds a sensible size for the item which should allow for best ergonomy.
         *          According to the policy one can determine whether some extra space can be
         *          used (or conversely if some missing space is a problem) but it should be
         *          the target size of the item.
         */
        utils::Sizef m_sizeHint;

        /**
         * @brief - Holds a maximum size over which the item starts being unusable. Up until
         *          this value the item can make use of some extra space but not beyond. Such
         *          an area is the theoretical maximum bound for usability of this item.
         */
        utils::Sizef m_maxSize;

        /**
         * @brief - Defines the strategy of the item regarding space allocation. This allows
         *          for precise determination of the capability of the item to use some extra
         *          space or to determine whether it's a problem if the item has to be shrunk.
         *          This policy is best used in conjunction with the layout system, and is taken
         *          into consideration when computing the space to assign to the item.
         */
        SizePolicy m_sizePolicy;

        /**
         * @brief - Defines the strategy of the item to handle focus. The focus action allows
         *          the item to intercept specific user's actions if it is activated and prevent
         *          other elements from getting these actions.
         *          This is useful both for performing specific operations on such actions and also
         *          improve performance by filtering these actions from being sent to other elements
         *          of the `ui`.
         *          Several level of awareness of the focus can be defined, the default being that
         *          no focus actions are transmitted to the item.
         */
        FocusPolicy m_focusPolicy;

        /**
         * @brief - Used to determine whether the geometry information held by this item is
         *          up to date. This is particularly useful to delay geometry computations to
         *          a later date, for example when an event of type `geometry update` is received.
         *          Ideally whenever a request to retrieve size information for this item is
         *          received, it should be checked against this status to trigger a recomputation
         *          if it appears that the information is not up to date.
         */
        bool m_geometryDirty;

        /**
         * @brief - Describes the current rendering area assigned to this item. Should always
         *          be greater than the `m_minSize`, smaller than the `m_maxSize` and as close
         *          to the `m_sizeHint` (if any is provided).
         *          This is used in computation to allocate and fill the internal visual textures
         *          used to represent the item.
         */
        utils::Boxf m_area;

        /**
         * @brief - Used to determine whether the item is visible or not. This is used by layouts
         *          for example where visibility can determine if an item will get some space or
         *          not.
         */
        bool        m_visible;

        /**
         * @brief - Used to prevent concurrent accesses to the above `m_visible` boolean.
         */
        mutable std::mutex  m_visibleLocker;

        /***
         * @brief - Allows to determine whether this item has the focus or not. The item
         *          can gain focus for a variety of reasons which include (but not limit to)
         *          whenever the mouse enters the item's boundaries, when a click is made
         *          inside the item's boundaries or when it gets focused by repeatingly
         *          pressing the `Tab` key until the tab chain reach this item.
         */
        FocusState  m_focusState;

        /**
         * @brief - The z order for this layout item. The z order allows for specific items to be
         *          drawn after some other items so that we get some sort of overlapping behavior.
         *          Basically in the case of a combobox for example, the item might extend beyond
         *          its assigned area, thus overlapping with other children items.
         *          In order to guarantee that such items get drawn after all the other children
         *          and thus get full advantage of their extended representation, one can use
         *          the z order.
         *          The z order's default value is 0. Any value higher than that will give priority
         *          to the item that has it (e.g. a component with a z order of `1` will be displayed
         *          in front of a component with a z order of `0`).
         */
        int m_zOrder;

        /**
         * @brief - Describes whether this item has the keyboard focus or not. The keyboard focus is
         *          received whenever the internal state of the item allows it. Some items are never
         *          able to receive the keyboard focus if the policy does not allow it.
         *          Usually a focus reason of `Tab` or `Click` is sufficient to gain the keyboard focus.
         */
        bool m_keyboardFocus;

        /**
         * @brief - A pointer to the layout into which this item might be inserted. Most of
         *          the time a layout item is not used alone byut rather inserted into a
         *          hierarchy of elements where each layer manage the size and position of
         *          the layers beneath it. In specific cases where this item is a top-level
         *          item, this attribute might reference the application's layout into which
         *          this item has been inserted.
         */
        LayoutItem* m_manager;
    };

    using LayoutItemShPtr = std::shared_ptr<LayoutItem>;
  }
}

# include "LayoutItem.hxx"

#endif    /* LAYOUT_ITEM_HH */
