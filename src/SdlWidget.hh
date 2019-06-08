#ifndef    SDLWIDGET_HH
# define   SDLWIDGET_HH

# include <mutex>
# include <memory>
# include <unordered_map>

# include <maths_utils/Box.hh>
# include <maths_utils/Size.hh>
# include <maths_utils/Vector2.hh>
# include <core_utils/Uuid.hh>
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

        utils::Boxf
        getRenderingArea() const noexcept override;

        void
        setVisible(bool visible) noexcept override;

        void
        setLayout(std::shared_ptr<Layout> layout) noexcept;

        void
        setPalette(const engine::Palette& palette) noexcept;

        void
        setEngine(engine::EngineShPtr engine) noexcept;

        virtual utils::Uuid
        draw();

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
        mouseMoveEvent(const engine::MouseEvent& e) override;

        bool
        repaintEvent(const engine::PaintEvent& e) override;

        bool
        resizeEvent(const engine::ResizeEvent& e) override;

        unsigned
        getChildrenCount() const noexcept;

        bool
        hasLayout() const noexcept;

        template <typename WidgetType>
        WidgetType*
        getChildAs(const std::string& name);

        template <typename LayoutType>
        LayoutType*
        getLayoutAs() noexcept;

        const engine::Palette&
        getPalette() const noexcept;

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

        using WidgetMap = std::unordered_map<std::string, SdlWidget*>;

      private:

        WidgetMap m_children;
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
    };

    using SdlWidgetShPtr = std::shared_ptr<SdlWidget>;
  }
}

# include "SdlWidget.hxx"

#endif    /* SDLWIDGET_HH */
