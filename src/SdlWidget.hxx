#ifndef    SDLWIDGET_HXX
# define   SDLWIDGET_HXX

# include "SdlWidget.hh"

# include <core_utils/CoreWrapper.hh>
# include "BoxUtils.hh"
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
    utils::maths::Sizef
    SdlWidget::getMinSize() const noexcept {
      std::lock_guard<std::mutex> guard(m_drawingLocker);
      return m_minSize;
    }

    inline
    utils::maths::Sizef
    SdlWidget::getSizeHint() const noexcept {
      std::lock_guard<std::mutex> guard(m_drawingLocker);
      return m_sizeHint;
    }

    inline
    utils::maths::Sizef
    SdlWidget::getMaxSize() const noexcept {
      std::lock_guard<std::mutex> guard(m_drawingLocker);
      return m_maxSize;
    }

    inline
    SizePolicy
    SdlWidget::getSizePolicy() const noexcept {
      std::lock_guard<std::mutex> guard(m_drawingLocker);
      return m_sizePolicy;
    }

    inline
    void
    SdlWidget::setMinSize(const utils::maths::Sizef& size) noexcept {
      std::lock_guard<std::mutex> guard(m_drawingLocker);
      m_minSize = size;
      makeGeometryDirty();
    }

    inline
    void
    SdlWidget::setSizeHint(const utils::maths::Sizef& hint) noexcept {
      std::lock_guard<std::mutex> guard(m_drawingLocker);
      m_sizeHint = hint;
      makeGeometryDirty();
    }

    inline
    void
    SdlWidget::setMaxSize(const utils::maths::Sizef& size) noexcept {
      std::lock_guard<std::mutex> guard(m_drawingLocker);
      m_maxSize = size;
      makeGeometryDirty();
    }

    inline
    utils::maths::Boxf
    SdlWidget::getRenderingArea() const noexcept {
      std::lock_guard<std::mutex> guard(m_drawingLocker);
      return m_area;
    }

    inline
    void
    SdlWidget::setRenderingArea(const utils::maths::Boxf& area) noexcept {
      utils::core::Logger::getInstance().logDebug(
        std::string("[") + getName() + "] Area is now (" +
        std::to_string(area.x()) + "x" + std::to_string(area.y()) +
        ", dims: " + std::to_string(area.w()) + "x" + std::to_string(area.h()) +
        ")",
        std::string("widget")
      );

      std::lock_guard<std::mutex> guard(m_drawingLocker);
      m_area = area;
      makeContentDirty();
    }

    inline
    void
    SdlWidget::setBackgroundColor(const Color& color) noexcept {
      std::lock_guard<std::mutex> guard(m_drawingLocker);
      m_palette.setBackgroundColor(color);
      makeContentDirty();
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
    SdlWidget::setLayout(std::shared_ptr<Layout> layout) noexcept {
      std::lock_guard<std::mutex> guard(m_drawingLocker);
      m_layout = layout;
      makeGeometryDirty();
    }

    inline
    void
    SdlWidget::setSizePolicy(const SizePolicy& policy) noexcept {
      std::lock_guard<std::mutex> guard(m_drawingLocker);
      m_sizePolicy = policy;
      makeGeometryDirty();
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

        SDL_Color bgColor = m_palette.getBackgroundColor()();

        if (event.y < 0) {
          if (bgColor.a < 10) {
            bgColor.a = 0;
          }
          else {
            bgColor.a = std::max(0, bgColor.a - 10);
          }
        }
        else {
          if (bgColor.a > 245) {
            bgColor.a = 255;
          }
          else {
            bgColor.a = std::min(255, bgColor.a + 10);
          }
        }
        m_palette.setBackgroundColor(Color(bgColor));
        std::cout << "[WIG] " << getName() << " alpha: " << std::to_string(bgColor.a) << std::endl;
        makeContentDirty();
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
    SdlWidget::hasContentChanged() const noexcept {
      return m_contentDirty && m_isVisible;
    }

    inline
    bool
    SdlWidget::hasGeometryChanged() const noexcept {
      return m_geometryDirty && m_isVisible;
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
      const SDL_Rect areaAsRect = utils::sdl::toSDLRect(m_area);

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

      // Assign the custom blend mode.
      int retCode = SDL_SetTextureBlendMode(textureContent, m_blendMode);
      if (retCode != 0) {
        throw SdlException(std::string("Cannot set blend mode to ") + std::to_string(m_blendMode) + " for widget \"" + getName() + "\" (err: \"" + SDL_GetError() + "\")");
      }

      SDL_Color bgColor = m_palette.getBackgroundColor()();

      // Assign alpha modulation to this texture based on the background color.
      SDL_SetTextureAlphaMod(textureContent, bgColor.a);

      // Fill the texture with opaque background color, transparency being handled using alpha modulation.
      RendererState state(renderer);
      SDL_SetRenderTarget(renderer, textureContent);
      SDL_SetRenderDrawColor(renderer, bgColor.r, bgColor.g, bgColor.b, SDL_ALPHA_OPAQUE);
      SDL_RenderClear(renderer);

      // Return the texture.
      return textureContent;
    }

    inline
    void
    SdlWidget::clearContentPrivate(SDL_Renderer* renderer, SDL_Texture* texture) const noexcept {
      // Save the current state of the renderer: this will automatically handle restoring the state upon destroying this object.
      RendererState state(renderer);

      SDL_Color bgColor = m_palette.getBackgroundColor()();

      SDL_SetRenderTarget(renderer, texture);
      SDL_SetRenderDrawColor(renderer, bgColor.r, bgColor.g, bgColor.b, SDL_ALPHA_OPAQUE);
      SDL_RenderClear(renderer);
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
      if (m_parent != nullptr) {
        m_parent->addWidget(this);
      }
    }

    inline
    void
    SdlWidget::makeContentDirty() noexcept {
      log(std::string("Content is now dirty"));
      m_contentDirty = true;
    }

    inline
    void
    SdlWidget::makeGeometryDirty() noexcept {
      log(std::string("Geometry is now dirty"));
      makeContentDirty();
      m_geometryDirty = true;
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
    SdlWidget::log(const std::string& message,
                   const utils::core::Level& level) const noexcept
    {
      if (m_logger != nullptr) {
        m_logger->logMessage(
          level,
          message,
          getName()
        );
      }
    }

    inline
    const WidgetFactory*
    SdlWidget::getFactory() const noexcept {
      return m_factory;
    }

    inline
    void
    SdlWidget::addWidget(SdlWidget* widget) {
      std::lock_guard<std::mutex> guard(m_drawingLocker);

      // Check for null widget.
      if (widget == nullptr) {
        throw SdlException(std::string("Cannot add null widget to \"") + getName() + "\"");
      }

      // Check for duplicated widget
      if (m_children.find(widget->getName()) != m_children.cend()) {
        throw SdlException(std::string("Cannot add duplicated widget \"") + widget->getName() + "\" to \"" + getName() + "\"");
      }

      m_children[widget->getName()] = widget;
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
      // Protect against errors.
      utils::core::launchProtected(
        [renderer, &child]() {
          // Draw this object (caching is handled by the object itself).
          SDL_Texture* picture = child.draw(renderer);

          // Draw the picture at the corresponding place.
          const utils::maths::Boxf& render = child.getRenderingArea();
          SDL_Rect dstArea = utils::sdl::toSDLRect(render);

          if (picture != nullptr) {
            SDL_RenderCopy(renderer, picture, nullptr, &dstArea);
          }
        },
        std::string("draw_child"),
        std::string("widget")
      );
    }

  }
}

#endif    /* SDLWIDGET_HXX */
