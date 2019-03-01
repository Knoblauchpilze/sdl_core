#ifndef    SDLWIDGET_HH
# define   SDLWIDGET_HH

# include <mutex>
# include <memory>
# include <unordered_map>
# include <SDL2/SDL.h>
# include "Box.hh"
# include "Size.hh"
# include "Color.hh"
# include "Layout.hh"
# include "Palette.hh"
# include "SizePolicy.hh"
# include "EventListener.hh"

namespace sdl {
  namespace core {

    class SdlWidget: public EventListener {
      public:

        SdlWidget(const std::string& name,
                  const sdl::utils::Sizef& sizeHint = sdl::utils::Sizef(),
                  SdlWidget* parent = nullptr,
                  const bool transparent = false,
                  const Palette& palette = Palette());

        virtual ~SdlWidget();

        const std::string&
        getName() const noexcept;

        sdl::utils::Sizef
        getMinSize() const noexcept;

        sdl::utils::Sizef
        getSizeHint() const noexcept;

        sdl::utils::Sizef
        getMaxSize() const noexcept;

        SizePolicy
        getSizePolicy() const noexcept;

        void
        setMinSize(const sdl::utils::Sizef& size) noexcept;

        void
        setSizeHint(const sdl::utils::Sizef& hint) noexcept;

        void
        setMaxSize(const sdl::utils::Sizef& size) noexcept;

        sdl::utils::Boxf
        getRenderingArea() const noexcept;

        void
        setRenderingArea(const sdl::utils::Boxf& area) noexcept;

        void
        setBackgroundColor(const Color& color) noexcept;

        bool
        isVisible() const noexcept;

        void
        setVisible(bool isVisible) noexcept;

        SDL_BlendMode
        getBlendMode() const noexcept;

        virtual SDL_Texture*
        draw(SDL_Renderer* renderer);

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

        virtual SDL_Texture*
        createContentPrivate(SDL_Renderer* renderer) const;

        virtual void
        clearContentPrivate(SDL_Renderer* renderer, SDL_Texture* texture) const noexcept;

        virtual void
        drawContentPrivate(SDL_Renderer* renderer, SDL_Texture* texture) const noexcept;

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
        drawChild(SDL_Renderer* renderer, SdlWidget& child);

      protected:

        friend class Layout;

        using WidgetMap = std::unordered_map<std::string, SdlWidget*>;

        std::string m_name;

        SdlWidget* m_parent;
        sdl::utils::Sizef m_minSize;
        sdl::utils::Sizef m_sizeHint;
        sdl::utils::Sizef m_maxSize;
        sdl::utils::Boxf m_area;
        Palette m_palette;
        SDL_BlendMode m_blendMode;

        bool m_contentDirty;
        bool m_geometryDirty;
        bool m_isVisible;
        bool m_transparent;
        SDL_Texture* m_content;
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
