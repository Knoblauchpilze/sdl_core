#ifndef    SDLWIDGET_HH
# define   SDLWIDGET_HH

# include <mutex>
# include <memory>
# include <unordered_map>

# include <SDL2/SDL.h>

# include <maths_utils/Box.hh>
# include <maths_utils/Size.hh>
# include <sdl_engine/Color.hh>
# include <sdl_engine/Palette.hh>
# include <sdl_engine/Texture.hh>

# include "Layout.hh"
# include "SizePolicy.hh"
# include "EventListener.hh"

namespace sdl {
  namespace core {

    class SdlWidget: public EventListener {
      public:

        SdlWidget(const std::string& name,
                  const utils::Sizef& sizeHint = utils::Sizef(),
                  SdlWidget* parent = nullptr,
                  const bool transparent = false,
                  const engine::Palette& palette = engine::Palette());

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
        setBackgroundColor(const engine::Color& color) noexcept;

        bool
        isVisible() const noexcept;

        void
        setVisible(bool isVisible) noexcept;

        virtual engine::Texture::UUID
        draw();

        unsigned
        getWidgetsCount() const noexcept;

        void
        setLayout(std::shared_ptr<Layout> layout) noexcept;

        void
        setSizePolicy(const SizePolicy& policy) noexcept;

        void
        onKeyPressedEvent(const SDL_KeyboardEvent& keyEvent) override;

        void
        onKeyReleasedEvent(const SDL_KeyboardEvent& keyEvent) override;

        void
        onMouseMotionEvent(const SDL_MouseMotionEvent& mouseMotionEvent) override;

        void
        onMouseButtonPressedEvent(const SDL_MouseButtonEvent& mouseButtonEvent) override;

        void
        onMouseButtonReleasedEvent(const SDL_MouseButtonEvent& mouseButtonEvent) override;

        void
        onMouseWheelEvent(const SDL_MouseWheelEvent& event) override;

        void
        onQuitEvent(const SDL_QuitEvent& event) override;

      protected:

        // We assume that this widget is already locked when we enter this method.
        virtual bool
        hasContentChanged() const noexcept;

        // We assume that this widget is already locked when we enter this method.
        virtual bool
        hasGeometryChanged() const noexcept;

        virtual std::shared_ptr<engine::Texture::UUID>
        createContentPrivate() const;

        virtual void
        clearContentPrivate(const engine::Texture::UUID& uuid) const noexcept;

        virtual void
        drawContentPrivate(const engine::Texture::UUID& uuid) const noexcept;

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

        bool m_contentDirty;
        bool m_geometryDirty;
        bool m_isVisible;
        bool m_transparent;
        std::shared_ptr<engine::Texture::UUID> m_content;
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
