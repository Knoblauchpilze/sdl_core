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
                  const bool transparent = false,
                  const engine::Color& color = engine::Color());

        virtual ~SdlWidget();

        utils::Sizef
        getMinSize() const noexcept;

        utils::Sizef
        getSizeHint() const noexcept;

        utils::Sizef
        getMaxSize() const noexcept;

        SizePolicy
        getSizePolicy() const noexcept;

        void
        setMinSize(const utils::Sizef& size) noexcept;

        void
        setSizeHint(const utils::Sizef& hint) noexcept;

        void
        setMaxSize(const utils::Sizef& size) noexcept;

        utils::Boxf
        getRenderingArea() const noexcept;

        void
        setRenderingArea(const utils::Boxf& area) noexcept;

        void
        setPalette(const engine::Palette& palette) noexcept;

        bool
        isVisible() const noexcept;

        void
        setVisible(bool isVisible) noexcept;

        virtual utils::Uuid
        draw();

        unsigned
        getWidgetsCount() const noexcept;

        void
        setLayout(std::shared_ptr<Layout> layout) noexcept;

        void
        setSizePolicy(const SizePolicy& policy) noexcept;

        void
        setEngine(engine::EngineShPtr engine) noexcept;

      protected:

        // Reimplementation of the base method defined in `engine::EngineObject`:
        // using this method we know that the events filters have already been
        // applied and we can safely process the event `e`. The aim of this method
        // is to transmit the event to children until it has been accepted.
        bool
        handleEvent(engine::EventShPtr e) override;

        virtual bool
        onKeyPressedEvent(const engine::KeyEvent& keyEvent);

        virtual bool
        onKeyReleasedEvent(const engine::KeyEvent& keyEvent);

        virtual bool
        onMouseMotionEvent(const engine::MouseEvent& mouseMotionEvent);

        virtual bool
        onMouseButtonPressedEvent(const engine::MouseEvent& mouseButtonEvent);

        virtual bool
        onMouseButtonReleasedEvent(const engine::MouseEvent& mouseButtonEvent);

        virtual bool
        onMouseWheelEvent(const engine::MouseEvent& mouseWheelEvent);

        virtual bool
        onQuitEvent(const engine::QuitEvent& quitEvent);

        // We assume that this widget is already locked when we enter this method.
        virtual bool
        hasContentChanged() const noexcept;

        // We assume that this widget is already locked when we enter this method.
        virtual bool
        hasGeometryChanged() const noexcept;

        utils::Vector2f
        mapToGlobal(const utils::Vector2f& local) const noexcept;

        utils::Vector2f
        mapFromGlobal(const utils::Vector2f& global) const noexcept;

        virtual utils::Uuid
        createContentPrivate() const;

        virtual void
        clearContentPrivate(const utils::Uuid& uuid) const noexcept;

        virtual void
        drawContentPrivate(const utils::Uuid& uuid) const noexcept;

        void
        setParent(SdlWidget* parent);

        void
        makeContentDirty() noexcept;

        void
        makeGeometryDirty() noexcept;

        std::mutex&
        getLocker() noexcept;

        template <typename WidgetType>
        WidgetType*
        getChildAs(const std::string& name);

        template <typename LayoutType>
        LayoutType*
        getLayoutAs() noexcept;

        engine::Engine&
        getEngine() const;

        const engine::Palette&
        getPalette() const noexcept;

      private:

        void
        addWidget(SdlWidget* widget);

        void
        clearTexture();

        void
        drawChild(SdlWidget& child);

      protected:

        friend class Layout;

        using WidgetMap = std::unordered_map<std::string, SdlWidget*>;

        SdlWidget* m_parent;
        utils::Sizef m_minSize;
        utils::Sizef m_sizeHint;
        utils::Sizef m_maxSize;
        utils::Boxf m_area;
        engine::Palette m_palette;

        engine::EngineShPtr m_engine;

        bool m_contentDirty;
        bool m_geometryDirty;
        bool m_isVisible;
        bool m_transparent;
        utils::Uuid m_content;
        mutable std::mutex m_drawingLocker;

        WidgetMap m_children;

        SizePolicy m_sizePolicy;
        std::shared_ptr<Layout> m_layout;
    };

    using SdlWidgetShPtr = std::shared_ptr<SdlWidget>;
  }
}

# include "SdlWidget.hxx"

#endif    /* SDLWIDGET_HH */
