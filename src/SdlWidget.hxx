#ifndef    SDLWIDGET_HXX
# define   SDLWIDGET_HXX

# include "SdlWidget.hh"
# include "SdlException.hh"

namespace sdl {
  namespace core {

    inline
    SdlWidget::~SdlWidget() {
      std::lock_guard<std::mutex> guard(m_drawingLocker);
      clearTexture();
      
      for (WidgetMap::const_iterator widget = m_children.cbegin() ;
           widget != m_children.cend() ;
           ++widget)
      {
        if (widget->second != nullptr) {
          delete widget->second;
        }
      }
    }

    inline
    const std::string&
    SdlWidget::getName() const noexcept {
      return m_name;
    }

    inline
    Boxf
    SdlWidget::getRenderingArea() const noexcept {
      std::lock_guard<std::mutex> guard(m_drawingLocker);
      return m_area;
    }

    inline
    void
    SdlWidget::setRenderingArea(const Boxf& area) noexcept {
      std::lock_guard<std::mutex> guard(m_drawingLocker);
      m_area = area;
      makeDirty();
    }

    inline
    void
    SdlWidget::setBackgroundColor(const SDL_Color& color) noexcept {
      std::lock_guard<std::mutex> guard(m_drawingLocker);
      m_background = color;
      makeDirty();
    }

    inline
    bool
    SdlWidget::isDrawable() const noexcept {
      std::lock_guard<std::mutex> guard(m_drawingLocker);
      return m_isDrawable;
    }

    inline
    void
    SdlWidget::setDrawable(bool isDrawable) noexcept {
      std::lock_guard<std::mutex> guard(m_drawingLocker);
      m_isDrawable = isDrawable;
    }

    inline
    SDL_BlendMode
    SdlWidget::getBlendMode() const noexcept {
      std::lock_guard<std::mutex> guard(m_drawingLocker);
      return m_blendMode;
    }

    inline
    unsigned
    SdlWidget::getWidgetsCount() const noexcept {
      std::lock_guard<std::mutex> guard(m_drawingLocker);
      return m_children.size();
    }

    inline
    void
    SdlWidget::setLayout(std::shared_ptr<SdlLayout> layout) noexcept {
      std::lock_guard<std::mutex> guard(m_drawingLocker);
      m_layout = layout;
      makeDirty();
    }

    inline
    void
    SdlWidget::onKeyPressedEvent(const SDL_KeyboardEvent& keyEvent) {
      std::lock_guard<std::mutex> guard(m_drawingLocker);
      for (WidgetMap::const_iterator widget = m_children.cbegin() ;
           widget != m_children.cend() ;
           ++widget)
      {
        widget->second->onKeyReleasedEvent(keyEvent);
      }
    }

    inline
    void
    SdlWidget::onKeyReleasedEvent(const SDL_KeyboardEvent& keyEvent) {
      std::lock_guard<std::mutex> guard(m_drawingLocker);
      for (WidgetMap::const_iterator widget = m_children.cbegin() ;
           widget != m_children.cend() ;
           ++widget)
      {
        widget->second->onKeyReleasedEvent(keyEvent);
      }

      if (getName() == "left_widget" && keyEvent.keysym.sym == SDLK_KP_7) {
        m_background.a = std::min(m_background.a + 4, 255);
        std::cout << "[WIG] " << getName() << " alpha: " << std::to_string(m_background.a) << std::endl;
      }
      if (getName() == "left_widget" && keyEvent.keysym.sym == SDLK_KP_4) {
        m_background.a = std::max(m_background.a - 4, 0);
        std::cout << "[WIG] " << getName() << " alpha: " << std::to_string(m_background.a) << std::endl;
      }

      if (getName() == "right_widget" && keyEvent.keysym.sym == SDLK_KP_8) {
        m_background.a = std::min(m_background.a + 4, 255);
        std::cout << "[WIG] " << getName() << " alpha: " << std::to_string(m_background.a) << std::endl;
      }
      if (getName() == "right_widget" && keyEvent.keysym.sym == SDLK_KP_5) {
        m_background.a = std::max(m_background.a - 4, 0);
        std::cout << "[WIG] " << getName() << " alpha: " << std::to_string(m_background.a) << std::endl;
      }

      if (getName() == "widget4" && keyEvent.keysym.sym == SDLK_KP_9) {
        m_background.a = std::min(m_background.a + 4, 255);
        std::cout << "[WIG] " << getName() << " alpha: " << std::to_string(m_background.a) << std::endl;
      }
      if (getName() == "widget4" && keyEvent.keysym.sym == SDLK_KP_6) {
        m_background.a = std::max(m_background.a - 4, 0);
        std::cout << "[WIG] " << getName() << " alpha: " << std::to_string(m_background.a) << std::endl;
      }

      if (getName() == "widget5" && keyEvent.keysym.sym == SDLK_KP_1) {
        m_background.a = std::min(m_background.a + 4, 255);
        std::cout << "[WIG] " << getName() << " alpha: " << std::to_string(m_background.a) << std::endl;
      }
      if (getName() == "widget5" && keyEvent.keysym.sym == SDLK_KP_0) {
        m_background.a = std::max(m_background.a - 4, 0);
        std::cout << "[WIG] " << getName() << " alpha: " << std::to_string(m_background.a) << std::endl;
      }

      if (getName() == "widget6" && keyEvent.keysym.sym == SDLK_KP_2) {
        m_background.a = std::min(m_background.a + 4, 255);
        std::cout << "[WIG] " << getName() << " alpha: " << std::to_string(m_background.a) << std::endl;
      }
      if (getName() == "widget6" && keyEvent.keysym.sym == SDLK_KP_COMMA) {
        m_background.a = std::max(m_background.a - 4, 0);
        std::cout << "[WIG] " << getName() << " alpha: " << std::to_string(m_background.a) << std::endl;
      }
    }

    inline
    void
    SdlWidget::onMouseMotionEvent(const SDL_MouseMotionEvent& mouseMotionEvent) {
      std::lock_guard<std::mutex> guard(m_drawingLocker);
      for (WidgetMap::const_iterator widget = m_children.cbegin() ;
           widget != m_children.cend() ;
           ++widget)
      {
        widget->second->onMouseMotionEvent(mouseMotionEvent);
      }
    }

    inline
    void
    SdlWidget::onMouseButtonPressedEvent(const SDL_MouseButtonEvent& mouseButtonEvent) {
      std::lock_guard<std::mutex> guard(m_drawingLocker);
      for (WidgetMap::const_iterator widget = m_children.cbegin() ;
           widget != m_children.cend() ;
           ++widget)
      {
        widget->second->onMouseButtonPressedEvent(mouseButtonEvent);
      }
    }

    inline
    void
    SdlWidget::onMouseButtonReleasedEvent(const SDL_MouseButtonEvent& mouseButtonEvent) {
      std::lock_guard<std::mutex> guard(m_drawingLocker);
      for (WidgetMap::const_iterator widget = m_children.cbegin() ;
           widget != m_children.cend() ;
           ++widget)
      {
        widget->second->onMouseButtonReleasedEvent(mouseButtonEvent);
      }
    }

    inline
    void
    SdlWidget::onMouseWheelEvent(const SDL_MouseWheelEvent& event) {
      std::lock_guard<std::mutex> guard(m_drawingLocker);
      for (WidgetMap::const_iterator widget = m_children.cbegin() ;
           widget != m_children.cend() ;
           ++widget)
      {
        widget->second->onMouseWheelEvent(event);
      }
    }

    inline
    void
    SdlWidget::onQuitEvent(const SDL_QuitEvent& event) {
      std::lock_guard<std::mutex> guard(m_drawingLocker);
      for (WidgetMap::const_iterator widget = m_children.cbegin() ;
           widget != m_children.cend() ;
           ++widget)
      {
        widget->second->onQuitEvent(event);
      }
    }

    inline
    bool
    SdlWidget::hasChanged() const noexcept {
      return m_dirty && m_isDrawable;
    }

    inline
    SDL_Texture*
    SdlWidget::createContentPrivate(SDL_Renderer* renderer) const {
      const SDL_Rect areaAsRect = m_area.toSDLRect();

      SDL_Texture* content = SDL_CreateTexture(
        renderer,
        SDL_PIXELFORMAT_RGBA8888,
        SDL_TEXTUREACCESS_TARGET,
        areaAsRect.w,
        areaAsRect.h
      );

      int retCode = SDL_SetTextureBlendMode(content, m_blendMode);
      if (retCode != 0) {
        throw SdlException(std::string("Cannot set blend mode to ") + std::to_string(m_blendMode) + " for widget \"" + getName() + "\" (err: \"" + SDL_GetError() + "\")");
      }

      if (content == nullptr) {
        throw SdlException(std::string("Could not create content for widget \"") + getName() + "\" (err: \"" + SDL_GetError() + "\")");
      }

      // Clear the content (i.e. fill with color).
      clearContentPrivate(renderer, content);
      
      return content;
    }

    inline
    void
    SdlWidget::clearContentPrivate(SDL_Renderer* renderer, SDL_Texture* texture) const noexcept {
      SDL_Color currentDrawColor;
      SDL_GetRenderDrawColor(renderer, &currentDrawColor.r, &currentDrawColor.g, &currentDrawColor.b, &currentDrawColor.a);
      SDL_Texture* currentTarget = SDL_GetRenderTarget(renderer);
      SDL_BlendMode blendMode;
      SDL_GetRenderDrawBlendMode(renderer, &blendMode);

      SDL_SetRenderTarget(renderer, texture);
      SDL_SetRenderDrawBlendMode(renderer, m_blendMode);
      SDL_SetRenderDrawColor(renderer, m_background.r, m_background.g, m_background.b, m_background.a);
      SDL_RenderClear(renderer);

      SDL_SetRenderTarget(renderer, currentTarget);
      SDL_SetRenderDrawColor(renderer, currentDrawColor.r, currentDrawColor.g, currentDrawColor.b, currentDrawColor.a);
      SDL_SetRenderDrawBlendMode(renderer, blendMode);
    }

    inline
    void
    SdlWidget::drawContentPrivate(SDL_Renderer* renderer, SDL_Texture* texture) const noexcept {
      // Nothing to do.
    }

    inline
    void
    SdlWidget::setParent(SdlWidget* parent) {
      std::lock_guard<std::mutex> guard(m_drawingLocker);
      m_parent = parent;
    }

    inline
    void
    SdlWidget::makeDirty() noexcept {
      m_dirty = true;
    }

    inline
    std::mutex&
    SdlWidget::getLocker() noexcept {
      return m_drawingLocker;
    }

    template <typename WidgetType>
    inline
    WidgetType*
    SdlWidget::getChildAs(const std::string& name) {
      WidgetMap::const_iterator child = m_children.find(name);
      if (child == m_children.cend()) {
        throw SdlException(std::string("Cannot retrieve child widget ") + name + " in widget " + getName() + ", no such element");
      }
      return dynamic_cast<WidgetType*>(child->second);
    }

    template <typename LayoutType>
    inline
    LayoutType*
    SdlWidget::getLayoutAs() noexcept {
      return dynamic_cast<LayoutType*>(m_layout.get());
    }

    inline
    void
    SdlWidget::addWidget(SdlWidget* widget) {
      std::lock_guard<std::mutex> guard(m_drawingLocker);
      if (widget == nullptr) {
        throw SdlException(std::string("Cannot add null widget to \"") + getName() + "\"");
      }
      m_children[widget->getName()] = widget;

      if (m_layout != nullptr) {
        m_layout->addItem(widget);
      }
    }

    inline
    void
    SdlWidget::clearTexture() {
      if (m_content != nullptr) {
        SDL_DestroyTexture(m_content);
      }
    }

    inline
    void
    SdlWidget::drawChild(SDL_Renderer* renderer, SdlWidget& child) {
      // Draw this object (caching is handled by the object itself).
      SDL_Texture* picture = child.draw(renderer);

      // Draw the picture at the corresponding place.
      const Boxf& render = child.getRenderingArea();
      SDL_Rect dstArea = render.toSDLRect();

      if (picture != nullptr) {
        SDL_RenderCopy(renderer, picture, nullptr, &dstArea);
      }
    }

  }
}

#endif    /* SDLWIDGET_HXX */
