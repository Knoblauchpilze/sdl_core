#ifndef    SDLWIDGET_HXX
# define   SDLWIDGET_HXX

# include "SdlWidget.hh"

# include <core_utils/CoreWrapper.hh>

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
    utils::Sizef
    SdlWidget::getMinSize() const noexcept {
      std::lock_guard<std::mutex> guard(m_drawingLocker);
      return m_minSize;
    }

    inline
    utils::Sizef
    SdlWidget::getSizeHint() const noexcept {
      std::lock_guard<std::mutex> guard(m_drawingLocker);
      return m_sizeHint;
    }

    inline
    utils::Sizef
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
    SdlWidget::setMinSize(const utils::Sizef& size) noexcept {
      std::lock_guard<std::mutex> guard(m_drawingLocker);
      m_minSize = size;
      makeGeometryDirty();
    }

    inline
    void
    SdlWidget::setSizeHint(const utils::Sizef& hint) noexcept {
      std::lock_guard<std::mutex> guard(m_drawingLocker);
      m_sizeHint = hint;
      makeGeometryDirty();
    }

    inline
    void
    SdlWidget::setMaxSize(const utils::Sizef& size) noexcept {
      std::lock_guard<std::mutex> guard(m_drawingLocker);
      m_maxSize = size;
      makeGeometryDirty();
    }

    inline
    utils::Boxf
    SdlWidget::getRenderingArea() const noexcept {
      std::lock_guard<std::mutex> guard(m_drawingLocker);
      return m_area;
    }

    inline
    void
    SdlWidget::setRenderingArea(const utils::Boxf& area) noexcept {
      std::lock_guard<std::mutex> guard(m_drawingLocker);
      m_area = area;
      makeContentDirty();

      log(std::string("Area is now ") + m_area.toString());
    }

    inline
    void
    SdlWidget::setBackgroundColor(const engine::Color& color) noexcept {
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
    SdlWidget::setEngine(engine::EngineShPtr engine) noexcept {
      std::lock_guard<std::mutex> guard(m_drawingLocker);
      // Assign the engine to this widget.
      m_engine = engine;
      
      // Also: assign the engine to the children if any.
      for (WidgetMap::const_iterator widget = m_children.cbegin() ;
           widget != m_children.cend() ;
           ++widget)
      {
        widget->second->setEngine(engine);
      }

      makeContentDirty();
    }

    inline
    void
    SdlWidget::onKeyPressedEvent(const engine::KeyEvent& keyEvent) {
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
    SdlWidget::onKeyReleasedEvent(const engine::KeyEvent& keyEvent) {
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
    SdlWidget::onMouseMotionEvent(const engine::MouseEvent& mouseMotionEvent) {
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
    SdlWidget::onMouseButtonPressedEvent(const engine::MouseEvent& mouseButtonEvent) {
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
    SdlWidget::onMouseButtonReleasedEvent(const engine::MouseEvent& mouseButtonEvent) {
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
    SdlWidget::onMouseWheelEvent(const engine::MouseEvent& event) {
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
    SdlWidget::onQuitEvent(const engine::QuitEvent& event) {
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
    utils::Uuid
    SdlWidget::createContentPrivate() const {
      // So far we created the clear content texture which is stored in the
      // 'm_clearContent' texture and which contains a static texture filled
      // with the background color and made transaprent if needed.
      // We still need to create the render target we will use to draw children
      // of this widget, because we cannot do so on a static texture.
      // The aim of this method is thus to create an empty texture wiht correct
      // access so that we can copy the internal 'm_clearContent' texture onto it
      // and then draw children as well.

      // Create the texture using the engine. THe dmensions are retrieved from the
      // internal area.
      utils::Sizei size(static_cast<int>(m_area.w()), static_cast<int>(m_area.h()));
      utils::Uuid uuid = getEngine().createTexture(size);

      // Assign alpha modulation to this texture based on the background color.
      getEngine().setTextureAlpha(uuid, m_palette.getActiveColor());

      // Return the texture.
      return uuid;
    }

    inline
    void
    SdlWidget::clearContentPrivate(const utils::Uuid& uuid) const noexcept {
      // Use the engine to fill the texture with the color provided by the
      // internal palette. The state of the widget is stored in the palette
      // so it will automatically be handled by the engine.
      getEngine().fillTexture(uuid, m_palette);
    }

    inline
    void
    SdlWidget::drawContentPrivate(const utils::Uuid& /*uuid*/) const noexcept {
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
        error(std::string("Cannot retrieve child widget ") + name + ", no such element");
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
    engine::Engine&
    SdlWidget::getEngine() const {
      if (m_engine == nullptr) {
        error(std::string("Cannot retrieve null engine"));
      }

      return *m_engine;
    }

    inline
    void
    SdlWidget::addWidget(SdlWidget* widget) {
      std::lock_guard<std::mutex> guard(m_drawingLocker);

      // Check for null widget.
      if (widget == nullptr) {
        error(std::string("Cannot add null widget"), getName());
      }

      // Check for duplicated widget
      if (m_children.find(widget->getName()) != m_children.cend()) {
        error(std::string("Cannot add duplicated widget \"") + widget->getName() + "\"", getName());
      }

      // Assign the engine to this widget if none is assigned.
      if (widget->m_engine == nullptr) {
        widget->m_engine = m_engine;
      }

      m_children[widget->getName()] = widget;
    }

    inline
    void
    SdlWidget::clearTexture() {
      if (m_content.valid()) {
        getEngine().destroyTexture(m_content);
        m_content.invalidate();
      }
    }

    inline
    void
    SdlWidget::drawChild(SdlWidget& child) {
      const utils::Uuid& uuid = m_content;
      engine::Engine& engine = getEngine();

      // Protect against errors.
      withSafetyNet(
        [&child, &uuid, &engine]() {
          // Draw this object (caching is handled by the object itself).
          utils::Uuid picture = child.draw();

          // Draw the picture at the corresponding place.
          utils::Boxf render = child.getRenderingArea();
          engine.drawTexture(
            picture,
            &uuid,
            &render
          );
        },
        std::string("draw_child")
      );
    }

  }
}

#endif    /* SDLWIDGET_HXX */
