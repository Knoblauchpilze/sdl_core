#ifndef    SDLWIDGET_HH
# define   SDLWIDGET_HH

# include <mutex>
# include <chrono>
# include <memory>
# include <unordered_map>

# include <maths_utils/Box.hh>
# include <maths_utils/Size.hh>
# include <maths_utils/Vector2.hh>
# include <core_utils/Uuid.hh>
# include <core_utils/Signal.hh>
# include <sdl_engine/Color.hh>
# include <sdl_engine/Palette.hh>
# include <sdl_engine/Texture.hh>
# include <sdl_engine/Engine.hh>
# include <sdl_engine/KeyEvent.hh>
# include <sdl_engine/MouseEvent.hh>
# include <sdl_engine/QuitEvent.hh>

# include "Layout.hh"
# include "LayoutItem.hh"
# include "SizePolicy.hh"

namespace sdl {
  namespace core {

    class SdlWidget: public LayoutItem {
      public:

        SdlWidget(const std::string& name,
                  const utils::Sizef& sizeHint = utils::Sizef(),
                  SdlWidget* parent = nullptr,
                  const engine::Color& color = engine::Color());

        virtual ~SdlWidget();

        /**
         * @brief - Specialization of the base `LayoutItem` method which returns the
         *          area to use to represent this widget on the screen.
         *          It slightly differ from the rendering area in the sense that it
         *          is global instead of local to the parent widget.
         *          The fact that it is global allows to locate the widget using an
         *          absolute coordinate frame and thus abolish the need to draw its
         *          representation relatively to the parent.
         *          This is a corner stone on the strategy to draw widget larger than
         *          their expected size, such as in combobox for example.
         * @return - the bounding box representing the absolute position of this
         *           widget in the window coordinate frame with the dimensions of the
         *           rendering area.
         */
        utils::Boxf
        getDrawingArea() const noexcept override;

        /**
         * @brief - Returns the rendering area of this widget relatively to its parent
         *          widget. This area does take into account the transformation applied
         *          to this widget in the parent's coordinate frame but no more. It can
         *          be used by parent widgets to draw their children during the caching
         *          process where each child is repainted within a texture representing
         *          the parent.
         * @return - the area representing the position of this widget in the parent's
         *           coordinate frame.
         */
        utils::Boxf
        getRenderingArea() const noexcept override;

        void
        setVisible(bool visible) noexcept override;

        void
        setLayout(std::shared_ptr<Layout> layout) noexcept;

        const engine::Palette&
        getPalette() const noexcept;

        void
        setPalette(const engine::Palette& palette) noexcept;

        void
        setEngine(engine::EngineShPtr engine) noexcept;

        /**
         * @brief - Used to retrieve the identifier of the texture representing the
         *          content for this widget. If no valid identifier is available for
         *          this widget an error is raised.
         * @return - an identifier of a texture representing this widget.
         */
        virtual utils::Uuid
        getContentUuid();

        /**
         * @brief - Retrievs the z order for this widget.
         * @return - the z order for this widget.
         */
        int
        getZOrder() noexcept;

        /**
         * @brief - Used to determine whether this widget has received the keyboard focus.
         * @return - true if this widget has received keyboard focus, false otherwise.
         */
        bool
        hasKeyboardFocus() const noexcept;

        /**
         * @brief - Used to perform the rendering of this widget using the internal engine
         *          provided to it. This method mostly returns the cached texture to use
         *          for this widget.
         *          If some pending repaint operations are registered internally, they are
         *          also handled there as we know that this method will be called by the
         *          main thread. Most of the time this method will be really quick, but it
         *          might happen that it takes a bit longer if the cached content needs to
         *          be updated.
         *          Failure to draw the widget will raise an error.
         *          The return value corresponds to the index of the texture representing
         *          this widget.
         * @return - the index of the texture which has been produced by the drawing
         *           operation.
         */
        virtual utils::Uuid
        draw();

        /**
         * @brief - Attempts to draw the content of this widget on the provided `on` texture
         *          at the destination `dst`. The source area is represented using `src` arg
         *          and represents all the widget if it is left `null`.
         *          If no element of the hierarchy defined by this widget spans the input
         *          `src` area this method returns `false`. Otherwise it returns `true`.
         * @param on - an identifier representing where `this` widget should be displayed.
         * @param src - the source area of `this` widget should be displayed. This area is
         *              expressed in local coordinate frame.
         * @param dst - where the source area should be displayed on the `on` texture. Expressed
         *              in local `on` coordinate frame.
         * @return - `true` if the `src` area is spanned by an element of `this` widget's
         *           hierarchy and `false` otherwise.
         */
        virtual bool
        drawOn(const utils::Uuid& on,
               const utils::Boxf* src,
               const utils::Boxf* dst);

        /**
         * @brief - Reimplementation of the base `EngineObject` method which allows to
         *          filter out events for children widget in case this widget is made
         *          invisible.
         * @param watched - the object for which the filter should be applied.
         * @param e - the event to filter.
         * @return - true if the event should be filtered (i.e. not transmitted to the
         *           `watched` object) and false otherwise.
         */
        bool
        filterEvent(engine::EngineObject* watched,
                    engine::EventShPtr e) override;

        /**
         * @brief - Reimplementation of the `EngineObject` class method so
         *          that we also assign the events queue to the children
         *          widget if any.
         * @param queue - the events queue to assign to this widget.
         */
        void
        setEventsQueue(engine::EventsQueue* queue) noexcept override;

        /**
         * @brief - Assign the input `parent` as ancestor of this widget. The
         *          `addWidget` method will be called on the `parent` with this
         *          widget as argument unless the provided parent is null.
         * @param parent - the parent to assign to this widget.
         */
        void
        setParent(SdlWidget* parent);

      protected:

        /**
         * @brief - Used to mark the internal content as dirty. Updates the internal
         *          `m_contentDirty` flag which will indicate that the content needs
         *          a full reconstruction.
         *          Typical use case include a resize operation where the size of the
         *          widget has been modified and the content need to be rebuilt.
         *          This method also request a global repaint.
         */
        void
        makeContentDirty();

        /**
         * @brief - This method can be used to indicate that the content of the widget
         *          should be redrawn. Most of the time this indicates a modification
         *          in the structrue of the content displayed by the widget.
         *          Depending of the inheriting class it can mean a variety of things,
         *          from new widget displayed in selector widget to picture modified
         *          in picture widget.
         *          By default the user is not supposed to provide any area for which
         *          the content is provided (as `allArea` is true) but one can choose
         *          to specify that only part of the widget should be redrawn using
         *          the second parameter `area` and setting the value of `ællArea` to
         *          `false`.
         * @param allArea - true if the whole area of the widget should be redrawn
         *                  and false if only part of the content is dirty.
         *                  Note that if this value is `true` the second argument is
         *                  ignored.
         * @param area - the area of the content which should be redrawn. Allows to
         *               indicate that only part of the widget's content is actually
         *               dirty. This value is ignored if `allArea` is `true` and the
         *               whole widget's area is used instead.
         */
        void
        requestRepaint(const bool allArea = true,
                       const utils::Boxf& area = utils::Boxf()) noexcept;

        /**
         * @brief - Reimplementation of the base `LayoutItem` method to also invalidate the
         *          internal layout associated to this widget if any.
         *          Triggers a call to the base method nonetheless.
         */
        void
        makeGeometryDirty() override;

        /**
         * @brief - Reimplementation of the base `EngineObject` method to allow filtering
         *          of events based on the semantic provided by the widget class. Basically
         *          we know that some events will be ignored according to their position in
         *          the queue.
         *          For example whenever a Show event is registered, most events should be
         *          cancelled. In the same way whenever a `Resize` event is queued the other
         *          `GeometryUpdate` or `Repaint` should be cancelled as some will be re-
         *          created by the process.
         *          So no need to recompute them.
         */
        void
        trimEvents(std::vector<engine::EventShPtr>& events) override;

        /**
         * @brief - Reimplementation of the base class method to provide update
         *          of the layout for this widget if any.
         * @param window - the available size to perform the update.
         */
        void
        updatePrivate(const utils::Boxf& window) override;

        /**
         * @brief - Specialization of the base `EngineObject` method to allow a lock operation
         *          on this widget so that we protect concurrent access from drawing routine.
         * @param e - the event to handle.
         * @return - true if the event was recognized, false otherwise.
         */
        bool
        handleEvent(engine::EventShPtr e) override;

        bool
        enterEvent(const engine::EnterEvent& e) override;

        bool
        focusInEvent(const engine::FocusEvent& e) override;

        bool
        focusOutEvent(const engine::FocusEvent& e) override;

        bool
        gainFocusEvent(const engine::FocusEvent& e) override;

        bool
        leaveEvent(const engine::Event& e) override;

        bool
        lostFocusEvent(const engine::FocusEvent& e) override;

        bool
        mouseButtonReleaseEvent(const engine::MouseEvent& e) override;

        bool
        mouseMoveEvent(const engine::MouseEvent& e) override;

        /**
         * @brief - Reimplementation of the base `EngineObject` method to provide
         *          implementation for the repaint. The repaint method handles the
         *          creation of a content which can be used as the visual display
         *          of the widget. Such visual should be up-to-date with the content
         *          of the widget.
         *          In order to keep the cached content synchronized with the real
         *          content of the widget a refresh event is also issued at the end
         *          of this method.
         *          As the engine has some limitations which forbid the processing
         *          of such events outside the main thread, we have to save them
         *          internally.
         * @param e - the paint event to process.
         */
        bool
        repaintEvent(const engine::PaintEvent& e) override;

        bool
        resizeEvent(engine::ResizeEvent& e) override;

        bool
        zOrderChanged(const engine::Event& e) override;

        int
        getChildrenCount() const noexcept;

        /**
         * @brief - Attempts to remove the input `widget` from this widget.
         *          Note that if the widget is not valid an error is raised.
         * @param widget - the widget to remove.
         */
        void
        removeWidget(SdlWidget* widget);

        /**
         * @brief - Returns true if this widget has a layout managing its children's
         *          size and position, false otherwise.
         * @return - true if a layout is assigned to manage the children's size and
         *           position, false otherwise.
         */
        bool
        hasLayout() const noexcept;

        /**
         * @brief - Used to determine whether this widget has a parent or not. A
         *          widget is considered to have a parent if the `m_parent` field
         *          is not null.
         * @retrun - true if the `m_parent` field is not null (meaning that the
         *           widget has a parent) and false otherwise.
         */
        bool
        hasParent() const noexcept;

        /**
         * @brief - Used to determine whether `this` widget is an ancestor of
         *          the input `widget`. An ancestor is either the parent or the
         *          parent of its parent and so on.
         * @param widget - the potential descendant of `this` widget.
         * @return - `true` if `this` widget is an ancestor of the input `widget`,
         *           `false` otherwise.
         */
        bool
        isAncestor(const SdlWidget* widget) const noexcept;

        /**
         * @brief - Used to determine whether `this` widget is a descendant of
         *          the input `widget`. A descendant is either the child or the
         *          child of one of its children and so on.
         * @param widget - the potential ancestor of `this` widget.
         * @return - `true` if `this` widget is a descendant of the input `widget`,
         *           `false` otherwise.
         */
        bool
        isDescendant(const SdlWidget* widget) const noexcept;

        /**
         * @brief - Returns true if this widget has a child of any kind with a
         *          name matching the input string.
         *          Note that this method assumes that the `m_childrenLocker`
         *          mutex is already acquired.
         * @param name - the name of the child which should be searched.
         * @return - `true` if a child with the specified `name` exists in this
         *           widget and `false` otherwise.
         */
        bool
        hasChild(const std::string& name) const noexcept;

        /**
         * @brief - Try to retrieve the child with a name corresponding to the input
         *          string as a pointer to the specified object type.
         *          If no such child exist or if the child cannot be casted into the
         *          specified type an error is raised.
         * @param name - the name of the child to retrieve.
         * @return - the child with a name corresponding to the input string as a
         *           pointer to a `WidgetType` object.
         */
        template <typename WidgetType>
        WidgetType*
        getChildAs(const std::string& name) const;

        /**
         * @brief - Similar function of `getChildAs` but returns null if the item
         *          cannot be retrieved because it does not exist.
         *          It still raises an exception if the item is invalid.
         * @param name - the name of the widget to retrieve.
         * @return - a pointer to the widget with the specified name or null if the
         *           widget does not exist.
         */
        template <typename WidgetType>
        WidgetType*
        getChildOrNull(const std::string& name) const;

        template <typename LayoutType>
        LayoutType*
        getLayoutAs() noexcept;

        engine::Engine&
        getEngine() const;

        /**
         * @brief - Takes the input `local` vector in argument assuming it represents a
         *          position in local coordinate frame and transform it into global frame.
         *          If no parent is assigned to this widget the position is just offseted
         *          by the local rendering area. Otherwise the parent object is used to
         *          determine the local transformation applied to this widget in order to
         *          derive the global transformation.
         * @param local - the local vector to transform.
         * @return - the global equivalent of the local vector.
         */
        utils::Vector2f
        mapToGlobal(const utils::Vector2f& local) const noexcept;

        /**
         * @brief - Converts the input `global` vector in argument assuming it represents
         *          a position in the global coordinate frame and transform it into local
         *          frame.
         *          If no parent is assigned to this widget the position is just offseted
         *          with the local rendering area. Otherwise the parent obejct is used to
         *          determine the global transformation applied to this widget in order to
         *          derive the local transformation.
         * @param global - the global vector to transform
         * @return - the local equivalent of the global vector.
         */
        utils::Vector2f
        mapFromGlobal(const utils::Vector2f& global) const noexcept;

        /**
         * @brief - Similar behavior to `mapToGlobal` with a vector in argument but outputs
         *          and takes a box in argument.
         *          This method is a convenience method which transforms the center of the
         *          input box and creates a new box with the global version of the center
         *          of the `local` box and the same dimensions.
         *          The user can specify whether the mapping to global coordinate frame needs
         *          to account for the local position of the widget. This comes from the fact
         *          that some usage of this method are like so:
         *          `mapToGlobal(getRenderingArea())`.
         *          It seems like a pain to be able to call the method like so:
         *          `mapToGlobal(utils::Boxf(
         *              0,
         *              0,
         *              getRenderingArea().w(),
         *              getRenderingArea().h()
         *          ))`
         *          So the boolean allows to skip this part in the process. If the boolean is
         *          true we basically do not care about the position defined in the `local`
         *          argument and consider that the value is `[0; 0]`.
         * @param local - the box expressed in local coordinate frame to transform to global
         *                frame.
         * @return - the global equivalent of the local box.
         */
        utils::Boxf
        mapToGlobal(const utils::Boxf& local,
                    const bool accountForPosition = true) const noexcept;

        /**
         * @brief - Similar behavior to `mapFromGlobal` with a vector in argument but outputs
         *          and takes a box in argument.
         *          This method is a convenience method which transforms the center of the
         *          input box and creates a new box with the local version of the center
         *          of the `global` box and the same dimensions.
         * @param global - the box expressed in global coordinate frame to transform to local
         *                frame.
         * @return - the local equivalent of the global box.
         */
        utils::Boxf
        mapFromGlobal(const utils::Boxf& global) const noexcept;

        /**
         * @brief - Used to convert the input box expressed in relative coordinate frame
         *          relatively to this widget into an area expressed in a coordinate frame
         *          which can be used by the internal engine.
         *          This new coordinate frame is similar to the relative frame for this
         *          widget but the top left corner is at `[0, 0]`.
         * @param area - the area to convert. Note that this area should be expressed in a
         *               coordinate frame relative to this widget where the center is at
         *               `[0, 0]`.
         * @param reference - the reference area to use to performt he conversion.
         * @return - a converted version of the input `area` to a coordinate frame usable
         *           by the engine.
         */
        utils::Boxf
        convertToEngineFormat(const utils::Boxf& area,
                              const utils::Boxf& reference) const noexcept;

        /**
         * @brief - Used to convert the input `area` expressed in a coordinate frame similar
         *          to the `reference` into local coordinate frame.
         * @param area - the area to convert: this value is assumed to be expressed in a
         *               parent coordinate frame.
         * @param reference - the reference area to use to provide the local coordinate frame
         *                    into which the `area` should be converted.
         * @return - a converted area corresponding to the input `area` in a coordinate frame
         *           corresponding to the `reference`.
         */
        utils::Boxf
        convertToLocal(const utils::Boxf& area,
                       const utils::Boxf& reference) const noexcept;

        /**
         * @brief - Used to determine whether the mouse is currently inside this widget or not.
         * @return - true if the mouse is inside this widget, false otherwise.
         */
        bool
        isMouseInside() const noexcept;

        /**
         * @brief - Determine whether the input global position is blocked by any child of this
         *          widget. This method will try to acquire the lock on the children list so it
         *          should not be already lock upon calling this function.
         *          It might be used to filter some events based on whether they actually occur
         *          inside the widget and not the children.
         * @param global - the global position to check. It will be converted to local coordinate
         *                 frame internally.
         * @return - `true` if the global position is contained inside the rendering area of any
         *           child widget, `false` otherwise.
         */
        bool
        isBlockedByChild(const utils::Vector2f& global) const noexcept;

        /**
         * @brief - Used to apply all the pending graphical operations
         *          stored in the internal array. This array mostly
         *          contains pending repaint events which are applied
         *          by this method.
         *          This is mostly a convenience wrapper to loop on all
         *          the available events.
         */
        void
        handleGraphicOperations();

        /**
         * @brief - Separation of concern compared to the paint event. The
         *          `repaintEventPrivate` focuses on redrawing the part of
         *          the widget described by the update regions while this
         *          method focuses on notifying parent elements of the
         *          event and also rebuilding the cached content from the
         *          now up-to-date content.
         */
        void
        refreshPrivate(const engine::PaintEvent& e);

        /**
         * @brief - The specialization of the `repaintEvent` which is called
         *          upon actually processing the events. This method is triggered
         *          during the `draw` method and used to perform each individual
         *          event stored in the internal array.
         *          Note that calling this function in a thread different from
         *          the main thread will result in the creation of textures
         *          which cannot be used correctly for display purposes.
         * @param e - the repaint event to process.
         */
        void
        repaintEventPrivate(const engine::PaintEvent& e);

        /**
         * @brief - Base implementation of the create operation for this widget.
         *          The aim of this method is to create a texture which will be
         *          used as a base to draw content of the widget on it.
         *          The identifier of the created texture is returned so that it
         *          can be used right away.
         *          Inheriting classes can overload this method but in general
         *          it should not be useful. It is encouraged to rather overload
         *          the `drawContentPrivate` method, which is used to draw on the
         *          texture produced by this method.
         *          The user can specify the role with which the texture should be
         *          created. The default value is `Background` which indicates a
         *          default background color but one can choose any role.
         * @param role - the color role to assign to the texture upon creating it,
         *               default value being `Background`.
         * @return - the identifier of the texture which has been created.
         */
        virtual utils::Uuid
        createContentPrivate(const engine::Palette::ColorRole& role = engine::Palette::ColorRole::Background) const;

        /**
         * @brief - Base implementation of the clear operation for this widget.
         *          The clear operation includes refreshing the base canvas provided
         *          by this widget to inheriting classes to perform their visual
         *          interpretation.
         *          The canvas usually has a base color and other information are
         *          displayed on top of it. During the process of rendering the ui,
         *          some elements might be displayed on top of it, hiding it partially
         *          if not totally. When these elements are hidden/removed, the part
         *          behind it should be redrawn: this is performed by this method.
         *          The input arguments represents the base canvas to clear and the
         *          `area` represents the area of the canvas which should be clered.
         * @param uuid - an identifier provided by the internal engine representing
         *               the canvas to clear.
         * @param area - a box representing the area to clear. Note that the area
         *               might represent the entirety of the canvas.
         */
        virtual void
        clearContentPrivate(const utils::Uuid& uuid,
                            const utils::Boxf& area) const;

        /**
         * @brief - Base implementation of the drawing operation for this widget. The
         *          drtawing operation includes drawing all the additional content
         *          needed by inheriting classes on top of the base canvas. This method
         *          does nothing as no other information is available for the base
         *          widget but typically it is the method to overload to display a
         *          picture on top of the canvas for example. Or some text.
         *          This method is only called upon repainting the widget. The input
         *          arguments represents an identifier to retrieve the base canvas
         *          where we can draw and an area representing the portion of the base
         *          canvas to update.
         * @param uuid - an identifier provided by the internal enigne representing the
         *               canvas to drawn onto.
         * @param area - a box representing the area which should be redrawn. Note that
         *               the area might represent the entirety of the canvas.
         */
        virtual void
        drawContentPrivate(const utils::Uuid& uuid,
                           const utils::Boxf& area);

        /**
         * @brief - Proceeds to add the input `widget` as a child of this object.
         *          No automatic insertion in the layout is performed, but the
         *          utilities objects are shared (such as the events queue and
         *          the engine for instance).
         *          Inheriting classes may override this method to provide specific
         *          behavior upon inserting a widget, such as inserting it to a
         *          layout, modifying its background color, etc.
         *          This function is called by children widget when they're added
         *          to a widget as a child.
         *          Note that trying to add a null widget will raise an error.
         * @param widget - the widget to insert in this object.
         */
        virtual void
        addWidget(SdlWidget* widget);

        /**
         * @brief - Assigns a new z order for this widget. This method also notifies
         *          the parent widget to indicate that the z order for this widget
         *          has been modified.
         * @param order - the new z order for this widget.
         */
        void
        setZOrder(const int order);

        /**
         * @brief - Used to update the internal focus state from the input focus reason.
         *          Depending on the focus reason and the internal widget's policy for
         *          focus handling, we might or might not actually update anything in
         *          `this` widget's content.
         *          A typical action that is performed when the state of a widget is
         *          changed is to update the content's texture role to set it to a more
         *          appropriate value reflecting the change in focus of `this` widget.
         *          This action can be de/activated according to the focus policy. Some
         *          inheriting classes might also want to react to such changes by also
         *          update local attributes to better suited values: usually they do not
         *          want to specialize the update process but rather be notified about
         *          it.
         *          That's why this method is not directly set to virtual and rather
         *          triggers a call to the `stateUpdatedFromFocus` method if the change
         *          was meant to be changed. Inheriting classes are encouraged to overload
         *          rather that method compared to this one.
         * @param reason - the focus reason which triggered the update of the state in
         *                 the first place.
         * @param gainedFocus - `true` if this widget just gained focus, `false` if it
         *                      just lost the focus.
         */
        void
        updateStateFromFocus(const engine::FocusEvent::Reason& reason,
                             const bool gainedFocus);

        /**
         * @brief - Called by the `updateStateFromFocus` whenever the internal state has
         *          actually been updated. The input arguments provide more insight on the
         *          focus event which triggered the call to this method along with the
         *          current state of `this` widget.
         *          Inheriting classes are encouraged to overload this method in case some
         *          specific updates need to be performed upon changing the internal state
         *          of the widget.
         *          Note that this method is only called upon updating the state (i.e. we
         *          are sure that the state actually changed when this function is called)
         *          and when the focus reason which triggered the state update is handled
         *          by this widget (i.e. the focus policy allow reactions on this type of
         *          focus).
         * @param reason - the focus reason which triggered the state update.
         * @param gainedFocus - `true` if `this` widget just gained focus, `false` if it
         *                      lost the focus.
         */
        virtual void
        stateUpdatedFromFocus(const FocusState& state,
                              const bool gainedFocus);

        /**
         * @brief - Used to determine whether the input focus reason can trigger a keyboard
         *          focus modification (either gain or lost of said focus).
         *          This method can be specialized by inheriting classes in order to provide
         *          different keyboard focus behavior.
         *          It is called within the `updateStateFromFocus` method in order to determine
         *          whether the keyboard focus can be changed when the state is updated.
         * @param reason - the focus reason which should be checked for keyboard focus changes
         *                 capabilities.
         * @return - `true` if the input focus reason can trigger a keyboard focus change,
         *           `false` otherwise.
         */
        virtual bool
        canCauseKeyboardFocusChange(const engine::FocusEvent::Reason& reason) const noexcept;

        /**
         * @brief - Used to filter the input event if it is an instance of a mouse event.
         *          Such events need to be filtered carefully so that only the right child
         *          gets triggered with the corresponding handler.
         *          It is much easier for the parent to determine if a child is covering
         *          another one or somehow obstruct the transmission of events as it has
         *          intuitive notion of the siblings of a widget rather than to delegate
         *          this job to the child which would have a hard time gathering this info
         *          in the first place.
         *          The user needs to specify the child for which the event should be
         *          filtered. If the input `watched` cannot be casted into a valid widget
         *          this method returns false.
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
         * @brief - Used to filter the input event if it is an instance of a keyboard
         *          event. Such events need to be filtered carefully so that only the
         *          right child receives the event: we want to avoid unnecessary response
         *          of child when possible and only activates children which can actually
         *          process the event.
         *          Just like the `filterMouseEvents` method it is easier for the parent
         *          (which is installed as an event filter of any of its child) to filter
         *          the keyboard events rather than letting the child decide whether it
         *          should filter it. In addition to being more robust (because inheriting
         *          classes cannot change this behavior) it also provides more intuitive
         *          behavior as the parent knows about the children positions and is able
         *          to determine whether a child obstructs another.
         * @param watched - the element for which the keyboard event should be filtered.
         *                  If this method returns `false` the event `e` is not filtered
         *                  by this method and can be transmitted to the `watched` object
         *                  if all other checks passes.
         * @param e - the keyboard event which should be filtered. If the event is filtered
         *            by this method the return value is `true`.
         * @return - `true` if this event should be filtered and `false` otherwise.
         */
        virtual bool
        filterKeyboardEvents(const engine::EngineObject* watched,
                             const engine::KeyEventShPtr e) const noexcept;

      private:

        /**
         * @brief - Asks the engine to perform the needed operations to release the
         *          memory used by the internal `m_content` texture.
         *          Assumes that the `m_contentLocker` is already locked.
         *          No other texture is created.
         *          The return value indicates the color role assumed by the texture
         *          which is destroyed if any. If no valid texture were found the
         *          default `engine::Palette::ColorRole::Background` value is returned.
         * @return - the color role assumed by the texture destroyed by this method or
         *           `engine::Palette::ColorRole::Background` if no valid texture exists.
         */
        engine::Palette::ColorRole
        clearTexture();

        /**
         * @brief - Asks the engine to perform the needed operations to release the
         *          memory used by the internal `m_cachedContent` texture.
         *          Assumes that the `m_cacheLocker` is already locked.
         *          No other texture is created.
         */
        void
        clearCachedTexture();

        /**
         * @brief - Used to perform the rendering of the input `widget` element while
         *          providing a safety net in case the drawing fails and raises an
         *          error.
         *          Draws the input `src` area of the texture of the widget into the
         *          specified `dst` area of `this` widget's texture.
         * @param widget - the widget to draw.
         * @param src - the area of the `widget`'s texture to draw. Expressed in local
         *              texture coordinate frame.
         * @param dst - describes where the `src` area of the `widget` should be drawn
         *              on this widget. Expressed in `this` local widget's coordinate
         *              frame.
         */
        void
        drawWidget(SdlWidget& widget,
                   const utils::Boxf& src,
                   const utils::Boxf& dst);

        /**
         * @brief - Attempts to perform the rendering of the `src` area of the provided
         *          `widget` at the specified `dst` position of the `on` texture.
         * @param widget - the widget to display.
         * @param on - an identifier representing the texture onto which the `widget` should
         *             be drawn.
         * @param src - the source area of ths `widget`'s texture which should be drawn.
         * @param dst - where the content of the `widget` should be drawn.
         */
        void
        drawWidgetOn(SdlWidget& widget,
                     const utils::Uuid& on,
                     const utils::Boxf& src,
                     const utils::Boxf& dst);

        /**
         * @brief - Used to share the configuration data of this widget with the
         *          provided input widget.
         *          Configuration data include engine and events queue.
         *          Note that an error is raised if the widget is null.
         * @param widget - the widget with which configuration data should be shared.
         */
        void
        shareData(SdlWidget* widget);

        /**
         * @brief - Performs a rebuild of the z ordering of the children widgets. This
         *          method will sort the `m_children` array and then proceed to update
         *          the `m_names` with the corresponding indices.
         */
        void
        rebuildZOrdering();

      protected:

        friend class Layout;

        /**
         * @brief - Used to describe a children widget and its associated z order.
         *          The wrapper includes the widget itself and the z order applied
         *          to the widget.
         */
        struct ChildWrapper {
          SdlWidget* widget;
          int zOrder;

          /**
           * @brief - Builds a child wrapper with the specified widget and z order.
           * @param wid - the widget associated to this wrapper.
           * @param zOrder - the z order for this widget.
           */
          ChildWrapper(SdlWidget* wid,
                       const int zOrder = 0);

          /**
           * @brief - Performs the comparison of `this` with the `rhs` value. The
           *          comparison is performed on the `zOrder` of each element.
           * @param rhs - the element to compare with `this`.
           * @return - true if `this` is less than `rhs`, false otherwise.
           */
          bool
          operator<(const ChildWrapper& rhs) const noexcept;
        };

        using Timestamp = std::chrono::time_point<std::chrono::steady_clock>;

        using ChildrenMap = std::unordered_map<std::string, int>;
        using WidgetsMap = std::vector<ChildWrapper>;
        using RepaintMap = std::unordered_map<std::string, Timestamp>;

        using Guard = std::lock_guard<std::mutex>;

      private:

        /**
         * @brief - Contains all the children for this widget. Each widget is registered by its
         *          name and we prevent several items with the same name to be registered. Also
         *          the widgets are assigned a z order: by default it corresponds to the order
         *          in which they have been added to this item, but it can be specified by using
         *          the dedicated handler.
         *          In order to allow both for easy access to widgets based on their name and
         *          efficient drawing based on the z order, we use two distinct internal arrays.
         */
        ChildrenMap m_names;
        WidgetsMap m_children;

        /**
         * @brief - Describes the timestamps at which an area containing the children area has
         *          been processed by this widget. When used in conjunction with the repaint
         *          time of any child it allows to precisely determine which children need to
         *          be repainted. We can thus easily trash or keep repaint events coming from
         *          some particular child.
         */
        RepaintMap m_childrenRepaints;

        /**
         * @brief - Holds the timestamp of the last successful repaint for this widget. Allows
         *          to determine whether repaint events are relevant or not if they have been
         *          issued before the last repaint they can be trashed easily.
         */
        Timestamp m_repaint;

        /**
         * @brief - Used to protect the children maps from concurrent accesses.
         */
        mutable std::mutex m_childrenLocker;

        /**
         * @brief - The layout which handles positionning of children widget in the space for
         *          this widget. Basically the parent of this widget or the layout it is linked
         *          to will provide some available space to render this widget.
         *          This space can be used to draw children widgets. In order to allow for the
         *          children to be arranged in complex patterns the widget allows to define a
         *          layout to it. Each child widget will be registered to the layout and thus
         *          whenever the area assigned for this widget is changed, the layout will be
         *          recomputed so that children widgets get an up-to-date area.
         */
        std::shared_ptr<Layout> m_layout;

        /**
         * @brief - Describes the palette to use for this widget. A palette describes a set of
         *          colors to use for most state of the widget. Each state is associated to a
         *          certain set of actions. Typically when the user hover over the widget the
         *          background color might be changed to indicate this fact.
         */
        engine::Palette m_palette;

        /**
         * @brief - The main engine to use to perform processes related to the rendering api.
         *          Most features which require to interact with the low level rendering api
         *          uses this engine.
         *          Note that if no engine is provided most of the rendering will fail.
         */
        engine::EngineShPtr m_engine;

        /**
         * @brief - Represents the parent widget of this object. This parent allows to benefit
         *          from the rendering process and to share the data such as engine or events
         *          queue.
         */
        SdlWidget* m_parent;

        /**
         * @brief - Used to determine whether the rendering information held by this widget is up
         *          to date. This is particularly useful to delay repaint computations to a later
         *          date, for example when a call to `draw` is performed.
         *          Ideally whenever a request to retrieve rendering information for this widget
         *          is received, it should be checked against this status to trigger a repaint
         *          if it appears that the information is not up to date.
         */
        bool m_contentDirty;

        /**
         * @brief - True if the mouse cursor is currently hovering over this widget. False otherwise. This
         *          attribute is updated upon receiving `EnterEvent` and `LeaveEvent`.
         */
        bool m_mouseInside;

        /**
         * @brief - Describes whether this widget has the keyboard focus or not. The keyboard focus is
         *          received whenever the internal state of the widget allows it. Some widget are never
         *          able to receive the keyboard focus if the policy does not allow it.
         *          Usually a focus reason of `Tab` or `Click` is sufficient to gain the keyboard focus.
         */
        bool m_keyboardFocus;

        /**
         * @brief - The z order for this widget. The z order allows for specific widgets to be
         *          drawn after some other widgets so that we get some sort of overlapping
         *          behavior. basically in the case of a combobox for example, the widget might
         *          extend beyond its assigned range, thus overlapping with other children
         *          widgets.
         *          In order to guarantee that such widgets get drawn after all the other children
         *          and thus get full advantage of their extended representation, one can use
         *          the z order.
         *          The z order is only relevant for siblings widgets and there's no such thing
         *          as a global z ordering of widgets.
         */
        int m_zOrder;

        /**
         * @brief - Contains an identifier representing the current visual content associated to
         *          this widget. Such identifier is related to an underlying engine and allows to
         *          handle some sort of cache mechanism where the information is only recomputed
         *          upon receiving a repaint request.
         *          Note that this identifier may be invalid if no repaint have occurred yet.
         */
        utils::Uuid m_content;

        /**
         * @brief - Used to store internally the paint events to process upon calling the `draw` method.
         *          Due to some limitations in the engine we're using, we cannot create or use some
         *          textures outside of the main thread. This is a problem for caching and repainting
         *          in general so to avoid this problem we save the corresponding events internally in
         *          order to process them upon the next call to `draw` event.
         *          Usually only one paint or refresh event should be generated by the engine: if some
         *          additional ones are generated it should be merged together.
         *          Internally we keep only one instance of the event and perform an additional merging
         *          if needed. But this shouldn't be the case.
         */
        engine::PaintEventShPtr m_repaintOperation;

        /**
         * @brief - Used to protect the above identifier from concurrent accesses. Any application has
         *          two main loops running in parallel: the events loop and the rendering loop. Any
         *          modifications triggered by processing an event may or may not have an impact on the
         *          visual representation of the widget and we want to be sure that a repaint event is
         *          not processed while an event is updating the visual content of the widget. This is
         *          made possible by using this mutex in any situation where the `m_content` attribute
         *          can be modified.
         *          This mutex is also used to protect the access to the `m_repaintOperation` and the
         *          `m_refreshOperation` which directly modify the `m_content`. It makes sense to protect
         *          these attributes behind the same mutex.
         */
        mutable std::mutex m_contentLocker;

        /**
         * @brief - Containes the identifier of the texture currently cached for display purpose.
         *          While the container or one of its children is not modified it will be used
         *          when handling `draw` request.
         *          This value is protected by a separate mutex to allow for easy access to it even
         *          when the `m_content` is being updated simulatenously.
         *          One of the goal of the `repaintEvent` method is to create the `m_content` value
         *          and to copy it into the `m_cachedContent` so that it can be used in external
         *          `draw` requests.
         */
        utils::Uuid m_cachedContent;

        /**
         * @brief - Used to protect the `m_cachedContent` from concurrent access. This identifier can
         *          indeed be accessed when a `draw` request is handled or when the `repaintEvent`
         *          function finishes its process and perform the swap of the old content with the new
         *          one to cache.
         */
        mutable std::mutex m_cacheLocker;

      public:

        /**
         * @brief - This signal can be used for external objects to register whenever the widget is
         *          clicked. The widget will trigger the signal when this happens.
         */
        utils::Signal<const std::string&> onClick;
    };

    using SdlWidgetShPtr = std::shared_ptr<SdlWidget>;
  }
}

# include "SdlWidget.hxx"

#endif    /* SDLWIDGET_HH */
