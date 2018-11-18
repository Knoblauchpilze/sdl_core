#ifndef    SDLWIDGET_HH
# define   SDLWIDGET_HH

# include <mutex>
# include <memory>
# include <unordered_map>
# include <SDL2/SDL.h>
# include "Box.hh"
# include "SdlLayout.hh"
# include "SdlEventListener.hh"

namespace sdl {
  namespace core {

    using Boxf = sdl::utils::Box<float>;

    class SdlWidget: public SdlEventListener {
      public:

        SdlWidget(const std::string& name,
                  const Boxf& area,
                  SdlWidget* parent = nullptr,
                  const SDL_Color& backgroundColor = SDL_Color{0, 0, 0, SDL_ALPHA_OPAQUE});

        virtual ~SdlWidget();

        const std::string&
        getName() const noexcept;

        Boxf
        getRenderingArea() const noexcept;

        void
        setRenderingArea(const Boxf& area) noexcept;

        void
        setBackgroundColor(const SDL_Color& color) noexcept;

        bool
        isDrawable() const noexcept;

        void
        setDrawable(bool isDrawable) noexcept;

        SDL_BlendMode
        getBlendMode() const noexcept;

        virtual SDL_Texture*
        draw(SDL_Renderer* renderer);

        unsigned
        getWidgetsCount() const noexcept;

        void
        setLayout(std::shared_ptr<SdlLayout> layout) noexcept;

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
        onMouseWheelEvent(const SDL_MouseWheelEvent& event);

        void
        onQuitEvent(const SDL_QuitEvent& event);

      protected:

        // We assume that this widget is already locked when we enter this method.
        virtual bool
        hasChanged() const noexcept;

        virtual SDL_Texture*
        createContentPrivate(SDL_Renderer* renderer) const;

        virtual void
        clearContentPrivate(SDL_Renderer* renderer, SDL_Texture* texture) const noexcept;

        virtual void
        drawContentPrivate(SDL_Renderer* renderer, SDL_Texture* texture) const noexcept;

        void
        setParent(SdlWidget* parent);

        void
        makeDirty() noexcept;

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

        using WidgetMap = std::unordered_map<std::string, SdlWidget*>;

        std::string m_name;

        SdlWidget* m_parent;
        Boxf m_area;
        SDL_Color m_background;
        SDL_BlendMode m_blendMode;

        bool m_dirty;
        bool m_isDrawable;
        SDL_Texture* m_content;
        mutable std::mutex m_drawingLocker;

        WidgetMap m_children;

        std::shared_ptr<SdlLayout> m_layout;
    };

    using SdlWidgetShPtr = std::shared_ptr<SdlWidget>;

  }
}

# include "SdlWidget.hxx"

#endif    /* SDLWIDGET_HH */
