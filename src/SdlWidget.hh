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
# include <sdl_engine/EngineObject.hh>
# include <sdl_engine/KeyEvent.hh>
# include <sdl_engine/MouseEvent.hh>
# include <sdl_engine/QuitEvent.hh>

# include "Layout.hh"
# include "SizePolicy.hh"

namespace sdl {
  namespace core {

    class SdlWidget: public engine::EngineObject {
      public:

        SdlWidget(const std::string& name,
                  const utils::Sizef& sizeHint = utils::Sizef(),
                  SdlWidget* parent = nullptr,
                  const engine::Color& color = engine::Color());

        virtual ~SdlWidget();

        ///////////////////
        // Size handling //
        ///////////////////

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

        utils::Boxf
        getRenderingArea() const noexcept;

        void
        setRenderingArea(const utils::Boxf& area) noexcept;

        ///////////////////
        // Size handling //
        ///////////////////

        void
        setVisible(bool isVisible) noexcept;

        /////////////////////
        // Utility methods //
        /////////////////////

        void
        setLayout(std::shared_ptr<Layout> layout) noexcept;

        void
        setPalette(const engine::Palette& palette) noexcept;

        void
        setEngine(engine::EngineShPtr engine) noexcept;

        /////////////////////
        // Utility methods //
        /////////////////////

        virtual utils::Uuid
        draw();

      protected:

        ///////////////////
        // Size handling //
        ///////////////////

        void
        makeContentDirty() noexcept;

        void
        makeGeometryDirty() noexcept;

        ///////////////////
        // Size handling //
        ///////////////////

        /////////////////////
        // Events handling //
        /////////////////////

        /**
         * @brief -  Reimplementation of the base method defined in `engine::EngineObject`:
         *           using this method we know that the events filters have already been 
         *           applied and we can safely process the event `e`.
         *           This method adds transmission to the event received to children widget
         *           if it is not accepted by this widget.
         * @param e - the event to process.
         * @return - true if the event was recognized (should almost always be the case) and
         *           false otherwise.
         */
        bool
        handleEvent(engine::EventShPtr e) override;

        bool
        enterEvent(const engine::EnterEvent& e) override;

        bool
        geometryUpdateEvent(const engine::Event& e) override;

        bool
        leaveEvent(const engine::Event& e) override;

        bool
        mouseMoveEvent(const engine::MouseEvent& e) override;

        bool
        repaintEvent(const engine::PaintEvent& e) override;

        bool
        resizeEvent(const engine::ResizeEvent& e) override;

        /////////////////////
        // Events handling //
        /////////////////////

        /////////////////////
        // Utility methods //
        /////////////////////

        unsigned
        getChildrenCount() const noexcept;

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

        virtual bool
        hasGeometryChanged() const noexcept;

        utils::Vector2f
        mapToGlobal(const utils::Vector2f& local) const noexcept;

        utils::Vector2f
        mapFromGlobal(const utils::Vector2f& global) const noexcept;

        bool
        isVisible() const noexcept;

        bool
        isInsideWidget(const utils::Vector2f& global) const noexcept;

        bool
        isBlockedByChild(const utils::Vector2f& global) const noexcept;

        /////////////////////
        // Utility methods //
        /////////////////////

        ///////////////////////
        // Rendering methods //
        ///////////////////////

        virtual utils::Uuid
        createContentPrivate() const;

        virtual void
        clearContentPrivate(const utils::Uuid& uuid) const;

        virtual void
        drawContentPrivate(const utils::Uuid& uuid) const;

        ///////////////////////
        // Rendering methods //
        ///////////////////////

      private:

        void
        clearTexture();

        void
        setParent(SdlWidget* parent);

        void
        addWidget(SdlWidget* widget);

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

        /**
         * @brief - Describes the size policy for this widget. The policy is described using
         *          several sizes which roles are described below. In addition to that, the
         *          `m_sizePolicy` allows to determine the behavior of the widget when some
         *          extra space is allocated to it or if it should be srhunk for some reason.
         */
        /**
         * @brief - Holds the minimum size which can be assigned to a widget while still making
         *          the widget usable. Any area smaller than this would make the widget useless
         *          as the user would not be able to properly use it.
         */
        utils::Sizef m_minSize;

        /**
         * @brief - Holds a sensible size for the widget which should allow for best ergonomy.
         *          According to the policy one can determine whether some extra space can be
         *          used (or conversely if some missing space is a problem) but it should be
         *          the target size of the widget.
         */
        utils::Sizef m_sizeHint;

        /**
         * @brief - Holds a maximum size over which the widget starts being unusable. Up until
         *          this value the widget can make use of some extra space but not beyond. Such
         *          an area is the theoretical maximum bound for usability of this widget.
         */
        utils::Sizef m_maxSize;

        /**
         * @brief - Defines the strategy of the widget regarding space allocation. This allows
         *          for precise determination of the capability of the widget to use some extra
         *          space or to determine whether it's a problem if the widget has to be shrunk.
         *          This policy is best used in conjunction with the layout system, and is taken
         *          into consideration when computing the space to assign to the widget.
         */
        SizePolicy m_sizePolicy;

        /**
         * @brief - Used to determine whether the geometry information held by this widget is
         *          up to date. This is particularly useful to delay geometry computations to
         *          a later date, for example when an event of type `geometry update` is received.
         *          Ideally whenever a request to retrieve size information for this widget is
         *          received, it should be checked against this status to trigger a recomputation
         *          if it appears that the information is not up to date.
         */
        bool m_geometryDirty;

        /**
         * @brief - Describes the current rendering area assigned to this widget. Should always
         *          be greater than the `m_minSize`, smaller than the `m_maxSize` and as close
         *          to the `m_sizeHint` (if any is provided).
         *          This is used in computation to allocate and fill the internal visual textures
         *          used to represent the widget.
         */
        utils::Boxf m_area;

        bool m_isVisible;

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
