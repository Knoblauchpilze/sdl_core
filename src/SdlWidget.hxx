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
        widget->second->onKeyPressedEvent(keyEvent);
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

      if (getName() == "right_widget") {
        if (event.y < 0) {
          if (m_background.a < 10) {
            m_background.a = 0;
          }
          else {
            m_background.a = std::max(0, m_background.a - 10);
          }
        }
        else {
          if (m_background.a > 245) {
            m_background.a = 255;
          }
          else {
            m_background.a = std::min(255, m_background.a + 10);
          }
        }
        std::cout << "[WIG] " << getName() << " alpha: " << std::to_string(m_background.a) << std::endl;
        makeDirty();
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
      // So far we created the clear content texture which is stored in the
      // 'm_clearContent' texture and which contains a static texture filled
      // with the background color and made transaprent if needed.
      // We still need to create the render target we will use to draw children
      // of this widget, because we cannot do so on a static texture.
      // The aim of this method is thus to create an empty texture wiht correct
      // access so that we can copy the internal 'm_clearContent' texture onto it
      // and then draw children as well.

      // Retrieve the dimensions of the area to create.
      const SDL_Rect areaAsRect = m_area.toSDLRect();

      // Create the texture with correct access.
      SDL_Texture* textureContent = SDL_CreateTexture(
        renderer,
        SDL_PIXELFORMAT_RGBA8888,
        SDL_TEXTUREACCESS_TARGET,
        areaAsRect.w,
        areaAsRect.h
      );
      if (textureContent == nullptr) {
        throw SdlException(std::string("Could not create texture for widget \"") + getName() + "\" (err: \"" + SDL_GetError() + "\")");
      }

      // Assign alpha modulation to this texture based on the background color.
      SDL_SetTextureAlphaMod(textureContent, m_background.a);

      // Assign the blend mode.
      const SDL_BlendMode mode = (m_transparent ? m_transparentBlendMode : m_blendMode);

      int retCode = SDL_SetTextureBlendMode(textureContent, m_blendMode);
      if (retCode != 0) {
        throw SdlException(std::string("Cannot set blend mode to ") + std::to_string(mode) + " for widget \"" + getName() + "\" (err: \"" + SDL_GetError() + "\")");
      }

      // Return the texture.
      return textureContent;
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

    inline
    SDL_Texture*
    SdlWidget::createClearContent(SDL_Renderer* renderer) const {
      // What we want is to create a texture with a color corresponding to the internal
      // background color, and which is transparent if needed.
      // We cannot create a simple texture and then set a transparent key. To do so we need
      // to use the good'ol surface system to create a transparent one, and then transform it
      // into a texture.
      // Note that the texture created from a surface has static access, which means we cannot use
      // it as a render surface. This is not really a problem in our case because we use the
      // 'm_content' texture as a render target (which is created in the 'createContentPrivate'
      // method).

      // Retrieve the dimensions of the area to create.
      const SDL_Rect areaAsRect = m_area.toSDLRect();

      // Create the initial surface with the corresponding dimensions.
      SDL_Surface* surfaceContent = SDL_CreateRGBSurface(0, areaAsRect.w, areaAsRect.h, 32, 0, 0, 0, 0);
      if (surfaceContent == nullptr) {
        throw SdlException(std::string("Could not create content for widget \"") + getName() + "\" (err: \"" + SDL_GetError() + "\")");
      }

      // Fill the created surface with the background color.
      SDL_FillRect(surfaceContent, nullptr, SDL_MapRGB(surfaceContent->format, m_background.r, m_background.g, m_background.b));

      // Associate transparent color if needed.
      if (m_transparent) {
        SDL_SetColorKey(surfaceContent, SDL_TRUE, SDL_MapRGB(surfaceContent->format, m_background.r, m_background.g, m_background.b));
      }

      // Create a texture from this surface.
      SDL_Texture* staticTextureContent = SDL_CreateTextureFromSurface(renderer, surfaceContent);
      SDL_FreeSurface(surfaceContent);
      if (staticTextureContent == nullptr) {
        throw SdlException(std::string("Could not create static texture for widget \"") + getName() + "\" (err: \"" + SDL_GetError() + "\")");
      }

      // Return it.
      return staticTextureContent;
    }

  }
}

#endif    /* SDLWIDGET_HXX */
