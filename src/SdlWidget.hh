#ifndef    SDLWIDGET_HH
# define   SDLWIDGET_HH

# include <mutex>
# include <memory>
# include <unordered_map>
# include <SDL2/SDL.h>
# include "Box.hh"
# include "SdlLayout.hh"

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
        addWidget(std::shared_ptr<SdlWidget> child);

        virtual void
        removeWidget(std::shared_ptr<SdlWidget> child);

        virtual void
        removeWidget(const std::string& name);

        unsigned
        getWidgetsCount() const noexcept;

        void
        setLayout(std::shared_ptr<SdlLayout> layout) noexcept;

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
        mutable std::mutex m_drawingLocker;

        WidgetMap m_children;

        std::shared_ptr<SdlLayout> m_layout;
    };

    using SdlWidgetShPtr = std::shared_ptr<SdlWidget>;

  }
}

# include "SdlWidget.hxx"

#endif    /* SDLWIDGET_HH */
