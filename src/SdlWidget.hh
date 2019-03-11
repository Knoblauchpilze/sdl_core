#ifndef    SDLWIDGET_HH
# define   SDLWIDGET_HH

# include <mutex>
# include <memory>
# include <unordered_map>

# include <SDL2/SDL.h>

# include <maths_utils/Box.hh>
# include <maths_utils/Size.hh>
# include <core_utils/CoreLogger.hh>

# include "Color.hh"
# include "Layout.hh"
# include "Palette.hh"
# include "SizePolicy.hh"
# include "EventListener.hh"
# include "WidgetBuilder.hh"

namespace sdl {
  namespace core {

    class SdlWidget: public EventListener {
      public:

        virtual ~SdlWidget();

        const std::string&
        getName() const noexcept;

        utils::maths::Sizef
        getMinSize() const noexcept;

        utils::maths::Sizef
        getSizeHint() const noexcept;

        utils::maths::Sizef
        getMaxSize() const noexcept;

        SizePolicy
        getSizePolicy() const noexcept;

        void
        setMinSize(const utils::maths::Sizef& size) noexcept;

        void
        setSizeHint(const utils::maths::Sizef& hint) noexcept;

        void
        setMaxSize(const utils::maths::Sizef& size) noexcept;

        utils::maths::Boxf
        getRenderingArea() const noexcept;

        void
        setRenderingArea(const utils::maths::Boxf& area) noexcept;

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

        SdlWidget(const std::string& name,
                  const utils::maths::Sizef& sizeHint = utils::maths::Sizef(),
                  SdlWidget* parent = nullptr,
                  const bool transparent = false,
                  const Palette& palette = Palette(),
                  utils::core::LoggerShPtr logger = nullptr);

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

        void
        log(const std::string& message,
            const utils::core::Level& level = utils::core::Level::Debug) const noexcept;

      private:

        void
        addWidget(SdlWidget* widget);

        void
        clearTexture();

        void
        drawChild(SDL_Renderer* renderer, SdlWidget& child);

      protected:

        friend class Layout;
        friend class WidgetBuilder;

        using WidgetMap = std::unordered_map<std::string, SdlWidget*>;

        std::string m_name;

        SdlWidget* m_parent;
        utils::maths::Sizef m_minSize;
        utils::maths::Sizef m_sizeHint;
        utils::maths::Sizef m_maxSize;
        utils::maths::Boxf m_area;
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

        mutable utils::core::LoggerShPtr m_logger;
    };

    using SdlWidgetShPtr = std::shared_ptr<SdlWidget>;
  }
}

# include "SdlWidget.hxx"

#endif    /* SDLWIDGET_HH */
