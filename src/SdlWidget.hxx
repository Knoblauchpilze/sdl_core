#ifndef    SDLWIDGET_HXX
# define   SDLWIDGET_HXX

# include "SdlWidget.hh"

namespace sdl {
  namespace core {

    inline
    utils::Boxf
    SdlWidget::getRenderingArea() const noexcept {
      std::lock_guard<std::mutex> guard(m_drawingLocker);
      return LayoutItem::getRenderingArea();
    }

    inline
    bool
    SdlWidget::filterEvent(engine::EngineObject* /*watched*/,
                           engine::EventShPtr /*e*/)
    {
      // Check whether the widget is not visible.
      return !isVisible();
    }

    inline
    void
    SdlWidget::setEventsQueue(engine::EventsQueue* queue) noexcept {
      // Use the base handler to assign the events queue to this widget.
      LayoutItem::setEventsQueue(queue);

      // Assign the events queue to the layout if any.
      if (hasLayout()) {
        registerToSameQueue(m_layout.get());
      }

      // Also assign the queue to the children of this widget.
      for (WidgetMap::const_iterator widget = m_children.cbegin() ;
           widget != m_children.cend() ;
           ++widget)
      {
        registerToSameQueue(widget->second);
      }
    }

    inline
    void
    SdlWidget::setParent(SdlWidget* parent) {
      // Try to assign the parent if is not already the parent of this widget.
      if (m_parent == parent) {
        return;
      }

      // Assign the parent.
      m_parent = parent;

      // Share data with the parent.
      if (m_parent != nullptr) {
        m_parent->addWidget(this);
      }
    }

    inline
    void
    SdlWidget::makeContentDirty() noexcept {
      // Mark the content as dirty.
      m_contentDirty = true;

      // Trigger a geometry update event.
      postEvent(std::make_shared<engine::PaintEvent>(LayoutItem::getRenderingArea()));
    }

    inline
    void
    SdlWidget::makeGeometryDirty() {
      // Mark the geometry as dirty.
      LayoutItem::makeGeometryDirty();

      // Invalidate the layout if any.
      if (hasLayout()) {
        m_layout->makeGeometryDirty();
      }
    }

    inline
    void
    SdlWidget::updatePrivate(const utils::Boxf& window) {
      // Keep track of the old size.
      utils::Boxf old = LayoutItem::getRenderingArea();

      // Call parent method so that we stay up to date with latest
      // area.
      LayoutItem::updatePrivate(window);

      // Update the layout if any.
      if (hasLayout()) {
        postEvent(std::make_shared<engine::ResizeEvent>(window, old, m_layout.get()));
      }
    }

    inline
    void
    SdlWidget::setVisible(bool visible) noexcept {
      std::lock_guard<std::mutex> guard(m_drawingLocker);

      // Use the base handler to perform needed internal updates.
      LayoutItem::setVisible(visible);

      // Trigger a repaint event if the widget is set to visible.
      if (isVisible()) {
        makeContentDirty();
      }
    }

    inline
    void
    SdlWidget::setLayout(std::shared_ptr<Layout> layout) noexcept {
      // Save this layout into the internal attribute.
      m_layout = layout;

      // Share the events queue if needed.
      if (hasLayout()) {
        registerToSameQueue(m_layout.get());
      }

      // Install this widget as filter for the event of the layout.
      layout->installEventFilter(this);

      makeGeometryDirty();
    }

    inline
    const engine::Palette&
    SdlWidget::getPalette() const noexcept {
      return m_palette;
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

    inline
    unsigned
    SdlWidget::getChildrenCount() const noexcept {
      return m_children.size();
    }

    inline
    bool
    SdlWidget::hasLayout() const noexcept {
      return m_layout != nullptr;
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
    utils::Vector2f
    SdlWidget::mapToGlobal(const utils::Vector2f& local) const noexcept {
      // To transform `local` coordinate to global, we need to first
      // account for the `local` coordinate.
      utils::Vector2f global = local;

      utils::Boxf area = LayoutItem::getRenderingArea();

      // Now we need to account for the position of this widget.
      global.x() += area.x();
      global.y() += area.y();

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
      // As most of the conversion process is already handled in
      // engine, we don't have to handle anything here. The position
      // is already given according to the same coordinate frame used
      // by widgets: we only need to account for the position of the
      // widget in its parent.
      utils::Boxf area = LayoutItem::getRenderingArea();

      local.x() -= area.x();
      local.y() -= area.y();

      // This is the local representation of the input global position.
      return local;
    }

    inline
    bool
    SdlWidget::isInsideWidget(const utils::Vector2f& global) const noexcept {
      // Compute the local position of the mouse.
      utils::Vector2f local = mapFromGlobal(global);

      utils::Boxf area = LayoutItem::getRenderingArea();

      // In order to be inside the widget, the local mouse position should lie
      // within the range [-width/2 ; width/2] and [-height/2 ; height/2].
      return
        std::abs(local.x()) < area.w() / 2.0f &&
        std::abs(local.y()) < area.h() / 2.0f
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

    inline
    bool
    SdlWidget::handleEvent(engine::EventShPtr e) {
      // Lock the widget to prevent concurrent accesses.
      std::lock_guard<std::mutex> guard(m_drawingLocker);

      // Use and return the value provided by the base handler.
      return LayoutItem::handleEvent(e);
    }

    inline
    bool
    SdlWidget::enterEvent(const engine::EnterEvent& e) {
      // Update the role of the background texture if the item is not selected.
      if (getEngine().getTextureRole(m_content) == engine::Palette::ColorRole::Background) {
        getEngine().setTextureRole(m_content, engine::Palette::ColorRole::Highlight);
      }

      // The mouse is now inside this widget.
      m_mouseInside = true;

      log("Mouse entering");

      // Use base handler to determine whether the event was recognized.
      return engine::EngineObject::enterEvent(e);
    }

    inline
    bool
    SdlWidget::leaveEvent(const engine::Event& e) {
      // Update the role of the background texture if the item is not selected.
      if (m_content.valid() && getEngine().getTextureRole(m_content) == engine::Palette::ColorRole::Highlight) {
        getEngine().setTextureRole(m_content, engine::Palette::ColorRole::Background);
      }

      // The mouse is now outside this widget.
      m_mouseInside = false;

      log("Mouse leaving");

      // Use base handler to determine whether the event was recognized.
      return engine::EngineObject::leaveEvent(e);
    }

    inline
    bool
    SdlWidget::mouseButtonReleaseEvent(const engine::MouseEvent& e) {
      // Check whether the click occured inside the widget. If this is the case, we need
      // to update the role of the content to selected.
      // If the mouse is not inside the widget when the click occurs, we need to unset
      // selection of the item if any.

      if (m_content.valid()) {
        if (m_mouseInside) {
          getEngine().setTextureRole(m_content, engine::Palette::ColorRole::Dark);
        }
        else {
          getEngine().setTextureRole(m_content, engine::Palette::ColorRole::Background);
        }
      }

      // Use the base handler to determine whether the event was recognized.
      return engine::EngineObject::mouseButtonReleaseEvent(e);
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

    inline
    utils::Uuid
    SdlWidget::createContentPrivate() const {
      // Create the texture using the engine. The dmensions are retrieved from the
      // internal area.
      utils::Boxf area = LayoutItem::getRenderingArea();
      utils::Sizei size(static_cast<int>(area.w()), static_cast<int>(area.h()));
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

      // Install this object as filter for the child: this will allow to
      // filter out events in case the parent is made invisible.
      widget->installEventFilter(this);

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
