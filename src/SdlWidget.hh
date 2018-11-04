#ifndef    SDLWIDGET_HH
# define   SDLWIDGET_HH

# include <mutex>
# include <memory>
# include <unordered_map>
# include <SDL2/SDL.h>
# include "Box.hh"

namespace sdl {
  namespace core {

    using Boxf = sdl::utils::Box<float>;

    class SdlWidget {
      public:

        SdlWidget(const std::string& name,
                  const Boxf& area,
                  SdlWidget* parent = nullptr,
                  const SDL_Color& backgroundColor = SDL_Color{0, 0, 0, SDL_ALPHA_OPAQUE});

        virtual ~SdlWidget();

        const std::string&
        getName() const noexcept;

        const Boxf&
        getRenderingArea() const noexcept;

        void
        setRenderingArea(const Boxf& area) noexcept;

        void
        setBackgroundColor(const SDL_Color& color) noexcept;

        virtual SDL_Texture*
        draw(SDL_Renderer* renderer);

        virtual void
        addChild(std::shared_ptr<SdlWidget> child);

        virtual void
        removeChild(std::shared_ptr<SdlWidget> child);

        virtual void
        removeChild(const std::string& name);

        unsigned
        getChildCount() const noexcept;

      protected:

        // We assume that this widget is already locked when we enter this method.
        virtual bool
        hasChanged() const noexcept;

        virtual SDL_Texture*
        createContentPrivate(SDL_Renderer* renderer) const;

        virtual void
        clearContentPrivate(SDL_Renderer* renderer, SDL_Texture* texture) const noexcept;

        void
        setParent(SdlWidget* parent);

      private:

        void
        clearTexture();

        void
        drawChild(SDL_Renderer* renderer, SdlWidget& child);

      private:

        using WidgetMap = std::unordered_map<std::string, std::shared_ptr<SdlWidget>>;

        std::string m_name;

        SdlWidget* m_parent;
        Boxf m_area;
        SDL_Color m_background;
        mutable std::mutex m_propsLocker;

        bool m_dirty;
        SDL_Texture* m_content;
        std::mutex m_drawingLocker;

        WidgetMap m_children;
    };

    using SdlWidgetShPtr = std::shared_ptr<SdlWidget>;

  }
}

# include "SdlWidget.hxx"

#endif    /* SDLWIDGET_HH */
