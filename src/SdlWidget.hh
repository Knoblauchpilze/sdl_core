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
         * @brief - Returns the area to use to represent this widget on the screen.
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
        getDrawingArea() const noexcept;

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
         * @brief - Used to perform the rendering of this widget using the internal engine
         *          provided to it. This method mostly returns the cached texture to use
         *          for this widget.
         *          If some pending repaint and refresh operations they are also handled
         *          there as we know that this method will be called by the main thread.
         *          Most of the time this method will be really quick, but it might happen
         *          that it takes a bit longer due to some pending repaint operations to
         *          process.
         *          Failure to draw the widget will raise an error.
         *          The return value corresponds to the index of the texture representing
         *          this widget.
         * @return - the index of the texture which has been produced by the drawing
         *           operation.
         */
        virtual utils::Uuid
        draw();

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

        // Convenience define to describe the type of the `m_drawingLocker`.
        using LockerType = std::recursive_mutex;

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
         * @brief - Used to trigger a refresh operation. This will create a refresh
         *          event and post it on the local queue.
         *          A refresh operation consists into updating the content of the
         *          cached content so that it is up-to-date with the modifications
         *          possibly applied to the actual content.
         *          A typical use case is for highlighting of a widget.
         */
        void
        requestRefresh();

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
         * @bruef - Reimplementation of the base `EngineObject` method to provide
         *          a lock of the widget when processing events. This allows to
         *          prevent the rendering engine to access concurrently to the data
         *          of this widget while some events are being processed.
         *          No other specific behavior is added.
         *          Note that calling this method is equivalent to calling the base
         *          `EngineObject` method, it only adds the mutex overhead.
         * @param e - the event to handle.
         * @return - true if the event was recognized, false otherwise.
         */
        bool
        handleEvent(engine::EventShPtr e) override;

        bool
        enterEvent(const engine::EnterEvent& e) override;

        bool
        leaveEvent(const engine::Event& e) override;

        bool
        mouseButtonReleaseEvent(const engine::MouseEvent& e) override;

        bool
        mouseMoveEvent(const engine::MouseEvent& e) override;


        /**
         * @brief - Reimplementation of the base `EngineObject` method to provide
         *          implementation for the refresh. A widget should refresh its
         *          cached content upon performing the `refreshEvent`. As we cannot
         *          do so in a thread different from the main thread we have to save
         *          these events internally so that they can be processed upon
         *          calling the `draw` method.
         * @param e - the refresh event to proces.
         */
        bool
        refreshEvent(const engine::Event& e) override;

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

        template <typename WidgetType>
        WidgetType*
        getChildAs(const std::string& name);

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
        getChildOrNull(const std::string& name);

        template <typename LayoutType>
        LayoutType*
        getLayoutAs() noexcept;

        engine::Engine&
        getEngine() const;

        LockerType&
        getLocker() const noexcept;

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

        bool
        isInsideWidget(const utils::Vector2f& global) const noexcept;

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
         * @brief - The specialization of the `refreshEvent` which is called
         *          upon actually processing the events. This method is
         *          triggered during the `draw` method after all the repaint
         *          events have been processed.
         *          It basically consists in refreshing the cached content to
         *          make it match the actual content.
         */
        void
        refreshEventPrivate(const engine::Event& e);

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
         * @return - the identifier of the texture which has been created.
         */
        virtual utils::Uuid
        createContentPrivate() const;

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
                           const utils::Boxf& area) const;

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
         * @brief - Retrievs the z order for this widget.
         * @return - the z order for this widget.
         */
        int
        getZOrder() noexcept;

        /**
         * @brief - Assigns a new z order for this widget. This method also notifies
         *          the parent widget to indicate that the z order for this widget
         *          has been modified.
         * @param order - the new z order for this widget.
         */
        void
        setZOrder(const int order);

      private:

        /**
         * @brief - Asks the engine to perform the needed operations to release the
         *          memory used by the internal `m_content` texture.
         *          Assumes that the `m_drawingLocker` is already locked.
         *          No other texture is created.
         */
        void
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

        /**
         * @brief - Used to perform the rendering of the input `child` widget while
         *          providing a safety net in case the drawing fails and raises an
         *          error.
         *          This function will use the provided dimensions so that each child
         *          widget is drawn properly on the local canvas.
         * @param child - the child widget to draw.
         * @param dims - total size of the canvas into which the child is drawn.
         */
        void
        drawChild(SdlWidget& child,
                  const utils::Sizef& dims);

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
         * @brief - Used internally to hold the timestamp at which each child have been rebuilt.
         *          This helps triage some repaint events which might be generated by children
         *          and proceed only the ones which timestamp is greater than the last time the
         *          repaint was painted on the widget.
         */
        RepaintMap m_repaints;

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
         * @brief - Contains an identifier representing the current visual content associated to
         *          this widget. Such identifier is related to an underlying engine and allows to
         *          handle some sort of cache mechanism where the information is only recomputed
         *          upon receiving a repaint request.
         *          Note that this identifier may be invalid if no repaint have occurred yet.
         */
        utils::Uuid m_content;

        /**
         * @brief - Used to protect the above identifier from concurrent accesses. Any application has
         *          two main loops running in parallel: the events loop and the rendering loop. Any
         *          modifications triggered by processing an event may or may not have an impact on the
         *          visual representation of the widget and we want to be sure that a repaint event is
         *          not processed while an event is updating the visual content of the widget. This is
         *          made possible by using this mutex in any situation where the `m_content` attribute
         *          can be modified.
         *          We make the mutex recursive as it is used both in events handling and in the add
         *          child semantic. Doing so allows event to generate new insertion events.
         */
        // TODO: Should probably be modified back to simple mutex.
        mutable LockerType m_drawingLocker;

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
        engine::EventShPtr m_refreshOperation;

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

        /**
         * @brief - True if the mouse cursor is currently hovering over this widget. False otherwise. This
         *          attribute is updated upon receiving `EnterEvent` and `LeaveEvent`.
         */
        bool m_mouseInside;

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
