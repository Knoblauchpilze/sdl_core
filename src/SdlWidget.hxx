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
    SdlWidget::setPalette(const engine::Palette& palette) noexcept {
      std::lock_guard<std::mutex> guard(m_drawingLocker);
      m_palette = palette;
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
    bool
    SdlWidget::handleEvent(engine::EventShPtr e) {
      // Lock this widget to prevent concurrent modifications.
      std::lock_guard<std::mutex> guard(m_drawingLocker);

      // Check for degenerate event.
      if (e == nullptr) {
        // This should not happen.
        log(
          std::string("Dropping invalid null event"),
          utils::Level::Warning
        );

        // The event was not recognized.
        return false;
      }

      // Check the event type and dispatch to the corresponding handler.
      switch (e->getType()) {
        case core::engine::Event::Type::KeyPress:
          onKeyPressedEvent(*std::dynamic_pointer_cast<core::engine::KeyEvent>(e));
          break;
        case core::engine::Event::Type::KeyRelease:
          onKeyReleasedEvent(*std::dynamic_pointer_cast<core::engine::KeyEvent>(e));
          break;
        case core::engine::Event::Type::MouseMove:
          onMouseMotionEvent(*std::dynamic_pointer_cast<core::engine::MouseEvent>(e));
          break;
        case core::engine::Event::Type::MouseButtonPress:
          onMouseButtonPressedEvent(*std::dynamic_pointer_cast<core::engine::MouseEvent>(e));
          break;
        case core::engine::Event::Type::MouseButtonRelease:
          onMouseButtonReleasedEvent(*std::dynamic_pointer_cast<core::engine::MouseEvent>(e));
          break;
        case core::engine::Event::Type::MouseWheel:
          onMouseWheelEvent(*std::dynamic_pointer_cast<core::engine::MouseEvent>(e));
          break;
        case core::engine::Event::Type::Quit:
          onQuitEvent(*std::dynamic_pointer_cast<core::engine::QuitEvent>(e));
          break;
        default:
          // Event type is not handled, continue the process.
          break;
      }

      // Check whether the event has been accepted.
      if (e->isAccepted()) {
        // The event was obivously recognized.
        return true;
      }

      // Dispatch to children.
      WidgetMap::const_iterator widget = m_children.cbegin();

      while (widget != m_children.cend() && !e->isAccepted()) {
        widget->second->event(e);
        ++widget;
      }

      // Use the base handle to determine whether the event is recognized.
      return core::engine::EventListener::handleEvent(e);
    }

    inline
    bool
    SdlWidget::onKeyPressedEvent(const engine::KeyEvent& /*keyEvent*/) {
      // Empty implementation, assume the event was recognized.
      return true;
    }

    inline
    bool
    SdlWidget::onKeyReleasedEvent(const engine::KeyEvent& /*keyEvent*/) {
      // Empty implementation, assume the event was recognized.
      return true;
    }

    inline
    bool
    SdlWidget::onMouseMotionEvent(const engine::MouseEvent& mouseMotionEvent) {
      if (getName() == "left_widget") {
        utils::Vector2f local = mapFromGlobal(utils::Vector2f(mouseMotionEvent.getMousePosition()));
        log(
          std::string("Mouse is at ") + local.toString(),
          utils::Level::Info
        );
      }

      // Empty implementation, assume the event was recognized.
      return true;
    }

    inline
    bool
    SdlWidget::onMouseButtonPressedEvent(const engine::MouseEvent& /*mouseButtonEvent*/) {
      // Empty implementation, assume the event was recognized.
      return true;
    }

    inline
    bool
    SdlWidget::onMouseButtonReleasedEvent(const engine::MouseEvent& /*mouseButtonEvent*/) {
      // Empty implementation, assume the event was recognized.
      return true;
    }

    inline
    bool
    SdlWidget::onMouseWheelEvent(const engine::MouseEvent& /*mouseWheelEvent*/) {
      // Empty implementation, assume the event was recognized.
      return true;
    }

    inline
    bool
    SdlWidget::onQuitEvent(const engine::QuitEvent& /*quitEvent*/) {
      // Empty implementation, assume the event was recognized.
      return true;
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
    utils::Vector2f
    SdlWidget::mapToGlobal(const utils::Vector2f& local) const noexcept {
      // To transform `local` coordinate to global, we need to first
      // account for the `local` coordinate.
      utils::Vector2f global = local;

      // Now we need to account for the position of this widget.
      global.x() += m_area.x();
      global.y() += m_area.y();

      // Now we need to account for the transform applied to the parent
      // if any.
      if (m_parent != nullptr) {
        global = m_parent->mapToGlobal(global);
      }

      // This is the global representation of the input local position.
      return global;
    }

    inline
    utils::Vector2f
    SdlWidget::mapFromGlobal(const utils::Vector2f& global) const noexcept {
      // To transform `global` coordinate to local, we need to first
      // account for the `global` coordinate.
      utils::Vector2f local = global;

      // Account for the transformation applied to the parent
      // if any.
      if (m_parent != nullptr) {
        local = m_parent->mapFromGlobal(local);
      }

      // Now we need to account for the position of this widget.
      // While the `x` coordinate is straightforward because the SDL
      // does have the axis oriented in the same direction as our
      // local coordinate frame, it is not the case for the `y` axis
      // so we need to invert it.
      // The inversion of the y axis only stands when it has not yet
      // been performed, i.e. when the widget is at the top of its
      // hierarchy: otherwise its parent already handled it and we
      // should not do it again. If we invert each time we would
      // oscillate between valid and invalid coordinates.

      // `x` coordinate is straightforward.
      local.x() -= m_area.x();

      // `y` coordinate should be handled with care.
      if (m_parent == nullptr) {
        // No defined parent, let's invert the `y` axis.
        local.y() = m_area.y() - local.y();
      }
      else {
        // The inversion has already been handled, proceed
        // normally.
        local.y() -= m_area.y();
      }

      // This is the local representation of the input global position.
      return local;
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
      utils::Uuid uuid = getEngine().createTexture(size, engine::Palette::ColorRole::Background);

      // Assign alpha modulation to this texture based on the background color.
      getEngine().setTextureAlpha(uuid, m_palette.getBackgroundColor());

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
    const engine::Palette&
    SdlWidget::getPalette() const noexcept {
      return m_palette;
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

      // Copy also the internal area in order to perform the coordinate
      // frame transform.
      utils::Sizef dims = m_area.toSize();

      // Protect against errors.
      withSafetyNet(
        [&child, &uuid, &engine, &dims]() {
          // Draw this object (caching is handled by the object itself).
          utils::Uuid picture = child.draw();

          // Draw the picture at the corresponding place. Note that the
          // coordinates of the box of each child is in local coordinates
          // relatively to this widget.
          // In order to obtain good results, we need to convert to an
          // intermediate coordinate frame not centered on the origin but
          // rather on the position of this widget.
          // This is because the SDL talks in terms of top left corner
          // and we talk in terms of center.
          // The conversion cannot happen without knowing the dimension
          // of the input texture, which is only known here.
          utils::Boxf render = child.getRenderingArea();
          
          // Account for the intermediate coordinate frame transformation.
          render.x() += (dims.w() / 2.0f);
          render.y() = (dims.h() / 2.0f) - render.y();

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
