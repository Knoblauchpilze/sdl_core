#ifndef    SDLWIDGET_HH
# define   SDLWIDGET_HH

# include <mutex>
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

        void
        makeContentDirty() noexcept;

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

        bool
        repaintEvent(const engine::PaintEvent& e) override;

        bool
        resizeEvent(const engine::ResizeEvent& e) override;

        int
        getChildrenCount() const noexcept;

        /**
         * @brief - Attempts to remove the input `widget` from this widget.
         *          Note that if the widget is not valid an error is raised.
         * @param widget - the widget to remove.
         */
        void
        removeWidget(SdlWidget* widget);

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

        template <typename LayoutType>
        LayoutType*
        getLayoutAs() noexcept;

        engine::Engine&
        getEngine() const;

        std::mutex&
        getLocker() const noexcept;

        virtual bool
        hasContentChanged() const noexcept;

        utils::Vector2f
        mapToGlobal(const utils::Vector2f& local) const noexcept;

        utils::Vector2f
        mapFromGlobal(const utils::Vector2f& global) const noexcept;

        bool
        isInsideWidget(const utils::Vector2f& global) const noexcept;

        bool
        isBlockedByChild(const utils::Vector2f& global) const noexcept;

        virtual utils::Uuid
        createContentPrivate() const;

        virtual void
        clearContentPrivate(const utils::Uuid& uuid) const;

        virtual void
        drawContentPrivate(const utils::Uuid& uuid) const;

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

      private:

        void
        clearTexture();

        /**
         * @brief - Used to share the configuration data of this widget with the
         *          provided input widget.
         *          Configuration data include engine and events queue.
         *          Note that an error is raised if the widget is null.
         * @param widget - the widget with which configuration data should be shared.
         */
        void
        shareData(SdlWidget* widget);

        void
        drawChild(SdlWidget& child);

      protected:

        friend class Layout;

        using WidgetsMap = std::unordered_map<std::string, SdlWidget*>;

      private:

        WidgetsMap m_children;
        std::shared_ptr<Layout> m_layout;
        engine::Palette m_palette;
        engine::EngineShPtr m_engine;

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
         */
        mutable std::mutex m_drawingLocker;

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
