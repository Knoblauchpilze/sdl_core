#ifndef    SDLWIDGET_HXX
# define   SDLWIDGET_HXX

# include "SdlWidget.hh"

namespace sdl {
  namespace core {

    ///////////////////
    // Size handling //
    ///////////////////

    inline
    utils::Sizef
    SdlWidget::getMinSize() const noexcept {
      return m_minSize;
    }

    inline
    void
    SdlWidget::setMinSize(const utils::Sizef& size) noexcept {
      m_minSize = size;
      makeGeometryDirty();
    }

    inline
    utils::Sizef
    SdlWidget::getSizeHint() const noexcept {
      return m_sizeHint;
    }

    inline
    void
    SdlWidget::setSizeHint(const utils::Sizef& hint) noexcept {
      m_sizeHint = hint;
      makeGeometryDirty();
    }

    inline
    utils::Sizef
    SdlWidget::getMaxSize() const noexcept {
      return m_maxSize;
    }

    inline
    void
    SdlWidget::setMaxSize(const utils::Sizef& size) noexcept {
      m_maxSize = size;
      makeGeometryDirty();
    }

    inline
    SizePolicy
    SdlWidget::getSizePolicy() const noexcept {
      return m_sizePolicy;
    }

    inline
    void
    SdlWidget::setSizePolicy(const SizePolicy& policy) noexcept {
      m_sizePolicy = policy;
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
      postEvent(std::make_shared<engine::ResizeEvent>(area, m_area));
    }

    ///////////////
    // Internals //
    ///////////////

    inline
    void
    SdlWidget::makeContentDirty() noexcept {
      // Mark the content as dirty.
      m_contentDirty = true;

      // Trigger a geometry update event.
      postEvent(std::make_shared<engine::PaintEvent>(m_area));
    }

    inline
    void
    SdlWidget::makeGeometryDirty() noexcept {
      // Mark the geometry as dirty.
      m_geometryDirty = true;

      // Invalidate the layout if any.
      if (m_layout != nullptr) {
        m_layout->invalidate();
      }

      // Trigger a geometry update event.
      postEvent(std::make_shared<engine::Event>(engine::Event::Type::GeometryUpdate));
    }

    ///////////////////
    // Size handling //
    ///////////////////

    inline
    void
    SdlWidget::setVisible(bool isVisible) noexcept {
      // TODO: Should probably raise a 'hide/show' event ?
      std::lock_guard<std::mutex> guard(m_drawingLocker);

      // Set this widget as visible.
      m_isVisible = isVisible;

      // Trigger a repaint event if the widget is set to visible.
      if (m_isVisible) {
        makeContentDirty();
      }
    }

    /////////////////////
    // Utility methods //
    /////////////////////

    inline
    void
    SdlWidget::setLayout(std::shared_ptr<Layout> layout) noexcept {
      m_layout = layout;
      makeGeometryDirty();
    }

    inline
    void
    SdlWidget::setPalette(const engine::Palette& palette) noexcept {
      m_palette = palette;
      makeContentDirty();
    }

    inline
    void
    SdlWidget::setEngine(engine::EngineShPtr engine) noexcept {
      // Release the content of this widget if any.
      clearTexture();

      // Assign the engine to this widget.
      m_engine = engine;

      // Also: assign the engine to children widgets if any.
      for (WidgetMap::const_iterator widget = m_children.cbegin() ;
           widget != m_children.cend() ;
           ++widget)
      {
        widget->second->setEngine(engine);
      }

      makeContentDirty();
    }

    ///////////////
    // Internals //
    ///////////////

    inline
    unsigned
    SdlWidget::getChildrenCount() const noexcept {
      return m_children.size();
    }

    template <typename WidgetType>
    inline
    WidgetType*
    SdlWidget::getChildAs(const std::string& name) {
      WidgetMap::const_iterator child = m_children.find(name);
      if (child == m_children.cend()) {
        error(
          std::string("Cannot retrieve child widget ") + name,
          std::string("No such element")
        );
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
    const engine::Palette&
    SdlWidget::getPalette() const noexcept {
      return m_palette;
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
    std::mutex&
    SdlWidget::getLocker() const noexcept {
      return m_drawingLocker;
    }

    inline
    bool
    SdlWidget::hasContentChanged() const noexcept {
      return m_contentDirty && isVisible();
    }

    inline
    bool
    SdlWidget::hasGeometryChanged() const noexcept {
      return m_geometryDirty && isVisible();
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
    bool
    SdlWidget::isVisible() const noexcept {
      return m_isVisible;
    }

    inline
    bool
    SdlWidget::isInsideWidget(const utils::Vector2f& global) const noexcept {
      // Compute the local position of the mouse.
      utils::Vector2f local = mapFromGlobal(global);

      // In order to be inside the widget, the local mouse position should lie
      // within the range [-width/2 ; width/2] and [-height/2 ; height/2].
      return
        std::abs(local.x()) < m_area.w() / 2.0f &&
        std::abs(local.y()) < m_area.h() / 2.0f
      ;
    }

    inline
    bool
    SdlWidget::isBlockedByChild(const utils::Vector2f& global) const noexcept {
      // Compute the local position of the mouse.
      utils::Vector2f local = mapFromGlobal(global);

      // Traverse children and check whether one is on the way.
      for (WidgetMap::const_iterator child = m_children.cbegin() ; child != m_children.cend() ; ++child) {
        if (child->second->isVisible() && child->second->getRenderingArea().isInside(local)) {
          return true;
        }
      }

      // No widget on the way.
      return false;
    }

    /////////////////////
    // Utility methods //
    /////////////////////

    /////////////////////
    // Events handling //
    /////////////////////

    inline
    bool
    SdlWidget::enterEvent(const engine::EnterEvent& e) {
      // Update the role of the background texture.
      getEngine().setTextureRole(m_content, engine::Palette::ColorRole::Highlight);

      // The mouse is now inside this widget.
      m_mouseInside = true;

      log("Mouse entering");

      // Mark the event as accepted if it is directed only through this
      // object.
      if (isReceiver(e)) {
        e.accept();
      }

      // Use base handler to determine whether the event was recognized.
      return engine::EngineObject::enterEvent(e);
    }

    inline
    bool
    SdlWidget::leaveEvent(const engine::Event& e) {
      // Update the role of the background texture.
      getEngine().setTextureRole(m_content, engine::Palette::ColorRole::Background);

      // The mouse is now outside this widget.
      m_mouseInside = false;

      log("Mouse leaving");

      // Mark the event as accepted if it is directed only through this
      // object.
      if (isReceiver(e)) {
        e.accept();
      }

      // Use base handler to determine whether the event was recognized.
      return engine::EngineObject::leaveEvent(e);
    }

    inline
    bool
    SdlWidget::mouseMoveEvent(const engine::MouseEvent& e) {
      // Check whether the mouse is inside the widget and not blocked by any child.
      // Basically we want to trigger a `EnterEvent` whenever:
      // 1) The mouse was not inside the widget before.
      // 2) The mouse is not blocked by any widget.
      // And we want to trigger a `LeaveEvent` whenever:
      // 1) The mouse is not inside the widget anymore.
      // 2) The mouse is blocked by a child widget.

      const bool inside = isInsideWidget(e.getMousePosition());
      const bool blocked = isBlockedByChild(e.getMousePosition());

      if (m_mouseInside) {
        // We care about mouse being blocked by a child widget and by mouse leaving
        // the widget.
        if (!inside || blocked) {
          postEvent(std::make_shared<engine::Event>(engine::Event::Type::Leave));
        }
      }
      else {
        // We care about mouse entering the widget or blocking by child widget not
        // relevant anymore.
        if (inside && !blocked) {
          postEvent(
            std::make_shared<engine::EnterEvent>(
              e.getMousePosition()
            )
          );
        }
      }

      // Use base handler to determine whether the event was recognized.
      return engine::EngineObject::mouseMoveEvent(e);
    }

    /////////////////////
    // Events handling //
    /////////////////////

    ///////////////////////
    // Rendering methods //
    ///////////////////////

    inline
    utils::Uuid
    SdlWidget::createContentPrivate() const {
      // Create the texture using the engine. The dmensions are retrieved from the
      // internal area.
      utils::Sizei size(static_cast<int>(m_area.w()), static_cast<int>(m_area.h()));
      utils::Uuid uuid = getEngine().createTexture(size, engine::Palette::ColorRole::Background);

      // Return the texture.
      return uuid;
    }

    inline
    void
    SdlWidget::clearContentPrivate(const utils::Uuid& uuid) const {
      // Use the engine to fill the texture with the color provided by the
      // internal palette. The state of the widget is stored in the texture
      // through the color role. The corresponding color will be retrieved
      // from the palette to produce the corresponding rendering.
      getEngine().fillTexture(uuid, m_palette);
    }

    inline
    void
    SdlWidget::drawContentPrivate(const utils::Uuid& /*uuid*/) const {
      // Empty implementation.
    }

    ///////////////////////
    // Rendering methods //
    ///////////////////////

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
    SdlWidget::setParent(SdlWidget* parent) {
      m_parent = parent;
      if (m_parent != nullptr) {
        m_parent->addWidget(this);
      }
    }

    inline
    void
    SdlWidget::addWidget(SdlWidget* widget) {
      // Check for null widget.
      if (widget == nullptr) {
        error(std::string("Cannot add null widget"));
      }

      // Lock the widget to prevent concurrent modifications of the
      // internal children table.
      std::lock_guard<std::mutex> guard(m_drawingLocker);

      // Check for duplicated widget
      if (m_children.find(widget->getName()) != m_children.cend()) {
        error(std::string("Cannot add duplicated widget \"") + widget->getName() + "\"", getName());
      }

      // Share the data with this widget.
      shareData(widget);

      m_children[widget->getName()] = widget;
    }

    inline
    void
    SdlWidget::shareData(SdlWidget* widget) {
      // Check for null widget.
      if (widget == nullptr) {
        error(std::string("Cannot share data with null widget"));
      }

      // We need to assign the events queue to the child widget.
      registerToSameQueue(widget);

      // Assign the engine to this widget if none is assigned.
      if (widget->m_engine == nullptr) {
        widget->setEngine(m_engine);
      }
    }

  }
}

#endif    /* SDLWIDGET_HXX */
