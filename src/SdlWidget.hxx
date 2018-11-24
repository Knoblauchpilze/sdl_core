#ifndef    SDLWIDGET_HXX
# define   SDLWIDGET_HXX

# include "SdlWidget.hh"
# include "SdlException.hh"
# include "RendererState.hh"

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
    SdlWidget::isVisible() const noexcept {
      std::lock_guard<std::mutex> guard(m_drawingLocker);
      return m_isVisible;
    }

    inline
    void
    SdlWidget::setVisible(bool isVisible) noexcept {
      std::lock_guard<std::mutex> guard(m_drawingLocker);
      m_isVisible = isVisible;
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
      return m_dirty && m_isVisible;
    }

    inline
    SDL_Texture*
    SdlWidget::createContentPrivate(SDL_Renderer* renderer) const {
      const SDL_Rect areaAsRect = m_area.toSDLRect();

      // // Create the initial surface.
      SDL_Surface* rgbContent = SDL_CreateRGBSurface(0, areaAsRect.w, areaAsRect.h, 32, 0, 0, 0, 0);
      if (rgbContent == nullptr) {
        throw SdlException(std::string("Could not create rgb content for widget \"") + getName() + "\" (err: \"" + SDL_GetError() + "\")");
      }

      // Fill the created surface with the background color.
      SDL_FillRect(rgbContent, nullptr, SDL_MapRGBA(rgbContent->format, m_background.r, m_background.g, m_background.b, m_background.a));

      // Associate transparent color if needed.
      if (m_transparent) {
        SDL_SetColorKey(rgbContent, SDL_TRUE, SDL_MapRGBA(rgbContent->format, m_background.r, m_background.g, m_background.b, m_background.a));
      }
      SDL_SetSurfaceAlphaMod(rgbContent, m_background.a);

      SDL_Texture* rgbTexture = SDL_CreateTextureFromSurface(renderer, rgbContent);
      SDL_FreeSurface(rgbContent);
      if (rgbTexture == nullptr) {
        throw SdlException(std::string("Could not create rgb texture for widget \"") + getName() + "\" (err: \"" + SDL_GetError() + "\")");
      }

      // The texture is created using SDL_TEXTUREACCESS_STATIC, which means we cannot use it as a render target. Thus we cannot use
      // it directly, we first need to create a valid texture.
      SDL_Texture* content = SDL_CreateTexture(
        renderer,
        SDL_PIXELFORMAT_RGBA8888,
        SDL_TEXTUREACCESS_TARGET,
        areaAsRect.w,
        areaAsRect.h
      );
      if (content == nullptr) {
        throw SdlException(std::string("Could not create content for widget \"") + getName() + "\" (err: \"" + SDL_GetError() + "\")");
      }

      // Copy the texture to the one with valid access.
      RendererState state(renderer);

      // Perform the copy of the rgb texture to the output texture now that the transparency has been set.
      SDL_SetRenderTarget(renderer, content);
      SDL_RenderCopy(renderer, rgbTexture, nullptr, nullptr);
      SDL_DestroyTexture(rgbTexture);

      return content;
    }

    inline
    void
    SdlWidget::clearContentPrivate(SDL_Renderer* renderer, SDL_Texture* texture) const noexcept {
      // Save the current state of the renderer: this will automatically handle restoring the state upon destroying this object.
      RendererState state(renderer);

      SDL_SetRenderTarget(renderer, texture);
      SDL_RenderCopy(renderer, m_clearContent, nullptr, nullptr);
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
      if (m_clearContent != nullptr) {
        SDL_DestroyTexture(m_clearContent);
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
