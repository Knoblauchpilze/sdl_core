#ifndef    SDLWIDGET_HXX
# define   SDLWIDGET_HXX

# include "SdlWidget.hh"

namespace sdl {
  namespace core {

    inline
    SdlWidget::ChildWrapper::ChildWrapper(SdlWidget* wid,
                                          const int zOrder):
      widget(wid),
      zOrder(zOrder)
    {}

    inline
    bool
    SdlWidget::ChildWrapper::operator<(const ChildWrapper& rhs) const noexcept {
      return zOrder < rhs.zOrder;
    }

    inline
    utils::Boxf
    SdlWidget::getDrawingArea() const noexcept {
      // We need to retrieve the position of the parent and factor in its
      // position in order to compute the position of this widget.
      Guard guard(m_contentLocker);

      // Retrieve the internal box for this widget.
      utils::Boxf thisBox = LayoutItem::getDrawingArea();

      // Map the center to global coordinate.
      utils::Vector2f globalOffset = mapToGlobal(utils::Vector2f());

      // Compute final position from both boxes.
      return utils::Boxf(
        globalOffset.x(),
        globalOffset.y(),
        thisBox.w(),
        thisBox.h()
      );
    }

    inline
    utils::Boxf
    SdlWidget::getRenderingArea() const noexcept {
      // Lock this widget.
      Guard guard(m_contentLocker);

      // Return the value provided by the base handler.
      return LayoutItem::getRenderingArea();
    }

    inline
    void
    SdlWidget::setVisible(bool visible) noexcept {
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
      requestRepaint();
    }

    inline
    void
    SdlWidget::setEngine(engine::EngineShPtr engine) noexcept {
      // Release the content of this widget if any.
      clearTexture();

      // Assign the engine to this widget.
      m_engine = engine;

      // Also: assign the engine to children widgets if any.
      {
        Guard guard(m_childrenLocker);
        for (WidgetsMap::const_iterator child = m_children.cbegin() ;
            child != m_children.cend() ;
            ++child)
        {
          child->widget->setEngine(engine);
        }
      }

      makeContentDirty();
    }

    inline
    utils::Uuid
    SdlWidget::getContentUuid() {
      // Acquire the lock on the cached content uuid and return it.
      // If the cached content is not valid, raise an error.
      Guard guard(m_cacheLocker);

      if (!m_cachedContent.valid()) {
        error(std::string("Cannot get content uuid"), std::string("Invalid content uuid"));
      }

      return m_cachedContent;
    }

    inline
    int
    SdlWidget::getZOrder() noexcept {
      return m_zOrder;
    }

    inline
    bool
    SdlWidget::filterEvent(engine::EngineObject* watched,
                           engine::EventShPtr e)
    {
      // Check whether the widget is not visible or not active.
      if(!isVisible() || !isActive()) {
        return true;
      }

      // Handle mouse events filtering.
      if (filterMouseEvents(watched, e)) {
        return true;
      }

      // Check whether the parent filters it, in which case we
      // should filter it too.
      if (hasParent()) {
        return m_parent->filterEvent(this, e);
      }

      // TODO: Repaint of other widgets when they are displayed on siblings
      // of their parent does not work anymore.

      // The event is not filtered.
      return false;
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

      Guard guard(m_childrenLocker);
      // Also assign the queue to the children of this widget.
      for (WidgetsMap::const_iterator child = m_children.cbegin() ;
           child != m_children.cend() ;
           ++child)
      {
        registerToSameQueue(child->widget);
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
      if (hasParent()) {
        m_parent->addWidget(this);
      }
    }

    inline
    void
    SdlWidget::makeContentDirty() {
      // Mark the content as dirty.
      m_contentDirty = true;

      // Request a repaint event.
      requestRepaint();
    }

    inline
    void
    SdlWidget::requestRepaint(const bool allArea,
                              const utils::Boxf& area) noexcept
    {
      // Determine the area which should be updated: this will
      // indicate the type of event to create.
      utils::Boxf toRepaint = area;

      if (allArea) {
        // Check whether the area is valid.
        toRepaint = LayoutItem::getRenderingArea();

        if (!toRepaint.valid()) {
          // No valid area provided, do not post the event as nothing will
          // happen anyway.
          return;
        }
      }

      // Convert the area to repaint to global coordinate frame.
      utils::Boxf global = mapToGlobal(toRepaint, false);

      // Create the paint event.
      engine::PaintEventShPtr e = std::make_shared<engine::PaintEvent>(global);

      // Post it to trigger a content update.
      postEvent(e);
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
    bool
    SdlWidget::handleEvent(engine::EventShPtr e) {
      Guard guard(m_contentLocker);
      return LayoutItem::handleEvent(e);
    }

    inline
    int
    SdlWidget::getChildrenCount() const noexcept {
      return m_children.size();
    }

    inline
    void
    SdlWidget::removeWidget(SdlWidget* widget) {
      // Check whether this widget is valid.
      if (widget == nullptr) {
        error(
          std::string("Could not remove child widget from parent"),
          std::string("Invalid null child")
        );
      }

      Guard guard(m_childrenLocker);

      // Check whether we can find this widget in the internal table.
      ChildrenMap::const_iterator child = m_names.find(widget->getName());
      if (child == m_names.cend()) {
        error(
          std::string("Cannot remove widget \"") + widget->getName() + "\" from parent",
          std::string("No such item")
        );
      }

      if (child->second < 0 || child->second >= getChildrenCount()) {
        error(
          std::string("Cannot remove widget \"") + widget->getName() + "\" from parent",
          std::string("Item has invalid internal index ") + std::to_string(child->second) +
          " while only " + std::to_string(getChildrenCount()) + " are available"
        );
      }

      // Remove the widget from the children list.
      m_children.erase(m_children.begin() + child->second);

      // Remove the widget from the repaints' timestamps. We might fail to
      // find this widget if it has not been repainted at all. Weird but
      // not impossible.
      m_childrenRepaints.erase(widget->getName());

      // Rebuild the internal list of associations.
      rebuildZOrdering();
    }

    inline
    bool
    SdlWidget::hasLayout() const noexcept {
      return m_layout != nullptr;
    }

    inline
    bool
    SdlWidget::hasParent() const noexcept {
      return m_parent != nullptr;
    }

    inline
    bool
    SdlWidget::hasChild(const std::string& name) const noexcept {
      // Try to retrieve an iterator on the child.
      ChildrenMap::const_iterator child = m_names.find(name);

      // If we managed to find a child with a similar name we're good.
      return child != m_names.cend();
    }

    template <typename WidgetType>
    inline
    WidgetType*
    SdlWidget::getChildAs(const std::string& name) {
      // Use dedicated handler and raise an error if it
      // returns null.
      WidgetType* wid = getChildOrNull<WidgetType>(name);

      if (wid == nullptr) {
        error(
          std::string("Cannot retrieve child widget ") + name,
          std::string("No such element")
        );
      }

      return wid;
    }

    template <typename WidgetType>
    WidgetType*
    SdlWidget::getChildOrNull(const std::string& name) {
      Guard guard(m_childrenLocker);

      ChildrenMap::const_iterator child = m_names.find(name);
      if (child == m_names.cend()) {
        return nullptr;
      }

      if (child->second < 0 || child->second >= getChildrenCount()) {
        error(
          std::string("Cannot retrieve widget \"") + name + "\" in parent",
          std::string("Item has invalid internal index ") + std::to_string(child->second) +
          " while only " + std::to_string(getChildrenCount()) + " are available"
        );
      }

      return dynamic_cast<WidgetType*>(m_children[child->second].widget);
    }

    template <typename LayoutType>
    inline
    LayoutType*
    SdlWidget::getLayoutAs() noexcept {
      if (!hasLayout()) {
        error(
          std::string("Could not retrieve layout as \"") + typeid(LayoutType).name() + "\"",
          std::string("No layout assigned")
        );
      }

      LayoutType* out = dynamic_cast<LayoutType*>(m_layout.get());

      if (out == nullptr) {
        error(
          std::string("Could not retrieve layout as \"") + typeid(LayoutType).name() + "\"",
          std::string("Layout has incompatible type \"") + typeid(m_layout.get()).name() + "\""
        );
      }

      return out;
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
      if (hasParent()) {
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
      if (hasParent()) {
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
    utils::Boxf
    SdlWidget::mapToGlobal(const utils::Boxf& local,
                           const bool accountForPosition) const noexcept
    {
      return utils::Boxf(
        accountForPosition ? mapToGlobal(local.getCenter()) : mapToGlobal(utils::Vector2f()),
        local.w(),
        local.h()
      );
    }

    inline
    utils::Boxf
    SdlWidget::mapFromGlobal(const utils::Boxf& global) const noexcept {
      return utils::Boxf(
        mapFromGlobal(global.getCenter()),
        global.w(),
        global.h()
      );
    }

    inline
    utils::Boxf
    SdlWidget::convertToEngineFormat(const utils::Boxf& area,
                                     const utils::Boxf& reference) const noexcept
    {
      // Convert the input `area` by shifting the x axis by half the dimension and
      // by inverting the `y` axis.
      utils::Boxf converted = area;
      converted.x() += (reference.w() / 2.0f);
      converted.y() = (reference.h() / 2.0f) - area.y();

      // Return the converted area.
      return converted;
    }

    inline
    utils::Boxf
    SdlWidget::convertToLocal(const utils::Boxf& area,
                              const utils::Boxf& reference) const noexcept
    {
      // The position of the `reference` is used to convert the position of the
      // input `area`. To do so we compute the relative offset between both areas.
      // The dimensions are kept unchanged as there is no scaling.
      return utils::Boxf(
        area.x() - reference.x(),
        area.y() - reference.y(),
        area.w(),
        area.h()
      );
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
    SdlWidget::isMouseInside() const noexcept {
      return m_mouseInside;
    }

    inline
    bool
    SdlWidget::isBlockedByChild(const utils::Vector2f& global) const noexcept {
      Guard guard(m_childrenLocker);

      // Compute the local position of the mouse.
      utils::Vector2f local = mapFromGlobal(global);

      // Traverse children and check whether one is on the way.
      for (WidgetsMap::const_iterator child = m_children.cbegin() ; child != m_children.cend() ; ++child) {
        if (child->widget->isVisible() && child->widget->getRenderingArea().contains(local)) {
          return true;
        }
      }

      // No widget on the way.
      return false;
    }

    inline
    void
    SdlWidget::handleGraphicOperations() {
      // Lock the drawing locker in order to perform pending operations.
      Guard guard(m_contentLocker);

      // Perform both repaint and refresh operations registered internally.
      // We need to clear the existing pending operations before starting
      // the processing as new ones might be produced along the way.
      if (m_repaintOperation != nullptr) {
        engine::PaintEventShPtr e = m_repaintOperation;
        m_repaintOperation.reset();

        repaintEventPrivate(*e);
      }
    }

    inline
    bool
    SdlWidget::enterEvent(const engine::EnterEvent& e) {
      // Update the role of the background texture if the item is not selected.
      if (getEngine().getTextureRole(m_content) == engine::Palette::ColorRole::Background) {
        getEngine().setTextureRole(m_content, engine::Palette::ColorRole::Highlight);

        // Post a repaint event.
        requestRepaint();
      }

      // The mouse is now inside this widget.
      m_mouseInside = true;

      // We grabbed the focus, notify using an event if needed.
      postEvent(std::make_shared<engine::Event>(engine::Event::Type::FocusIn));

      // Use base handler to determine whether the event was recognized.
      return engine::EngineObject::enterEvent(e);
    }

    inline
    bool
    SdlWidget::focusInEvent(const engine::Event& e) {
      log("Handling focus in from " + e.getEmitter()->getName());

      // Set this widget as focused.
      setFocused(true);

      // Post a gain focus first to this widget (so that potential children
      // which are currently focused get deactivated) which will then be
      // transmitted to the parent so that it can do the necessary updates
      // regarding siblings of `this` widget which may be focused.
      postEvent(std::make_shared<engine::Event>(engine::Event::Type::GainFocus));

      // Use the base handler to provide a return value.
      return LayoutItem::focusInEvent(e);
    }

    inline
    bool
    SdlWidget::focusOutEvent(const engine::Event& e) {
      log("Handling focus out from " + e.getEmitter()->getName());

      // Set this widget as not focused.
      setFocused(false);

      // Post a lost focus first to this widget (so that potential children
      // which are currently focused get deactivated).
      postEvent(std::make_shared<engine::Event>(engine::Event::Type::LostFocus));

      // Use the base handler to provide a return value.
      return LayoutItem::focusOutEvent(e);
    }

    inline
    bool
    SdlWidget::gainFocusEvent(const engine::Event& e) {
      // This type of event is triggered by children widget in case they
      // just gained focus. The event's source should thus be a child
      // widget.
      // This method needs to unfocus any other child widget for `this`
      // widget which may have the focus and also notify the parent widget
      // or the manager layout if any with the fact that this widget has
      // now focus.
      // We also need to focus ourselves so that the chain of widgets
      // which lead to the deepest focused child can be built.
      log("Handling gain focus from " + e.getEmitter()->getName());

      // This widget now has the focus.
      setFocused(true);

      // Traverse the internal array of children and unfocus any widget
      // which is not the source of the event.
      {
        Guard guard(m_childrenLocker);
        for (WidgetsMap::const_iterator child = m_children.cbegin() ; child != m_children.cend() ; ++child) {

          log("Child " + child->widget->getName() + (child->widget->hasFocus() ? " has " : " has not ") + "focus");
          // If the child is not the source of the event and is focused, unfocus it.
          if (e.getEmitter() != child->widget && child->widget->hasFocus()) {
            log("Posting leave event on " + child->widget->getName() + " due to " + e.getEmitter()->getName() + " gaining focus");
            postEvent(std::make_shared<engine::Event>(engine::Event::Type::Leave, child->widget), false, true);
          }
        }
      }

      // Transmit the gain focus event to the parent widget if any or the
      // manager layout.
      engine::EventShPtr gfe = std::make_shared<engine::Event>(engine::Event::Type::GainFocus);
      EngineObject* o = nullptr;

      if (hasParent()) {
        gfe->setReceiver(m_parent);
        o = m_parent;
      }
      else if (isManaged()) {
        gfe->setReceiver(getManager());
        o = getManager();
      }

      if (o == nullptr) {
        log("Do not post gain focus event, no need to do so", utils::Level::Info);
      }

      if (o != nullptr) {
        postEvent(gfe, false, true);
      }

      // Use the base handler to provide a return value.
      return LayoutItem::gainFocusEvent(e);
    }

    inline
    bool
    SdlWidget::leaveEvent(const engine::Event& e) {
      // Update the role of the background texture if the item is not selected.
      if (m_content.valid() && getEngine().getTextureRole(m_content) == engine::Palette::ColorRole::Highlight) {
        getEngine().setTextureRole(m_content, engine::Palette::ColorRole::Background);

        // Post a repaint event.
        requestRepaint();
      }

      // The mouse is now outside this widget.
      m_mouseInside = false;

      // We lost the focus, notify using an event if needed.
      postEvent(std::make_shared<engine::Event>(engine::Event::Type::FocusOut));

      // Use base handler to determine whether the event was recognized.
      return engine::EngineObject::leaveEvent(e);
    }

    inline
    bool
    SdlWidget::lostFocusEvent(const engine::Event& e) {
      log("Handling lost focus from " + e.getEmitter()->getName());

      // A lost focus event comes after a leave event and means that the
      // focus has been removed from this widget. It also means that no
      // children can keep the focus, so we should transmit the leave event
      // to all the children of `this` widget.
      {
        Guard guard(m_childrenLocker);
        for (WidgetsMap::const_iterator child = m_children.cbegin() ; child != m_children.cend() ; ++child) {

          log("Child " + child->widget->getName() + (child->widget->hasFocus() ? " has " : " has not ") + "focus");
          // If the child is not the source of the event and is focused, unfocus it.
          if (child->widget->hasFocus()) {
            log("Posting leave event on " + child->widget->getName() + " due to " + getName() + " losing focus");
            postEvent(std::make_shared<engine::Event>(engine::Event::Type::Leave, child->widget), false, true);
          }
        }
      }

      // Use the base handler to provide a return value.
      return LayoutItem::lostFocusEvent(e);
    }

    inline
    bool
    SdlWidget::mouseButtonReleaseEvent(const engine::MouseEvent& e) {
      // Mouse events are only transmitted to this widget when the mouse is
      // inside the widget and if no other child block the view. We still
      // need to handle the repaint of the widget in case of a click and the
      // update in texture role.

      // Check whether the content for this widget is valid (i.e. at least
      // a repaint event has successfully been processed). If this is not
      // the case, return early.
      if (!m_content.valid()) {
        // Use the base handler to provide a return value.
        return engine::EngineObject::mouseButtonReleaseEvent(e);
      }

      // TODO: The repaint event does not occur because the mouse button
      // event is not transmitted to the widget which are not under the click
      // so no deselection.

      // We now need to determine whether we need a repaint: this is only
      // the case if it's the first occurrence of the click inside the
      // widget in which case we will update the texture's role.
      // If the widget already is selected just ignore the new click but
      // still fire the corresponding signal.
      bool needRepaint = false;

      // If the texture role is not already set to `Selected` (i.e. `Dark`
      // in engine's semantic) assign it.
      if (getEngine().getTextureRole(m_content) != engine::Palette::ColorRole::Dark) {
        getEngine().setTextureRole(m_content, engine::Palette::ColorRole::Dark);

        // We will need a repaint.
        needRepaint = true;
      }

      // Request a repaint event if needed.
      if (needRepaint) {
        requestRepaint();

        // Also produce a focus in event.
        postEvent(std::make_shared<engine::Event>(engine::Event::Type::FocusIn));
      }

      // Fire a signal indicating that a click on this widget has been detected.
      log("Emitting on click for " + getName(), utils::Level::Notice);
      onClick.emit(getName());
    
      // Use the base handler to provide a return value.
      return LayoutItem::mouseButtonReleaseEvent(e);
    }

    inline
    bool
    SdlWidget::mouseMoveEvent(const engine::MouseEvent& e) {
      // Mouse motion events are transmitted to the child as long
      // as it's inside the widget. This does not account for the
      // cases where a child is blocking the event and should thus
      // prevent `this` widget from reacting to it.
      // This can be checked by the `isBlockedByChild` function so
      // that we trigger a leave event as soon as the mouse enters
      // a child.

      // Check whether the mouse is blocked by a child.
      const bool blocked = isBlockedByChild(e.getMousePosition());

      // Now we need to track when the mouse will get blocked by
      // a child widget. Indeed the events system already filters
      // mouse events when the mouse is not inside `this` widget
      // so we only to handle production of enter and leave events
      // when the mouse gets blocked by a child.

      if (!isMouseInside()) {
        // If the mouse is not inside the widget (i.e. the mouse
        // just entered the widget) we need to post a `EnterEvent`
        // if the mouse is not blocked.
        if (!blocked) {
          postEvent(std::make_shared<engine::EnterEvent>(e.getMousePosition()));
        }
      }
      else {
        // If the mouse is already inside the widget (i.e. the mouse
        // entered the widget some time ago) we need to keep track
        // of when it gets blocked by any child, in which case we
        // need to post a leave event.
        if (blocked) {
          postEvent(std::make_shared<engine::Event>(engine::Event::Type::Leave));
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
      utils::Uuid uuid = getEngine().createTexture(area.toSize(), engine::Palette::ColorRole::Background);

      // Return the texture.
      return uuid;
    }

    inline
    void
    SdlWidget::clearContentPrivate(const utils::Uuid& uuid,
                                   const utils::Boxf& area) const
    {
      // Use the engine to fill the texture with the color provided by the
      // internal palette. The state of the widget is stored in the texture
      // through the color role. The corresponding color will be retrieved
      // from the palette to produce the corresponding rendering.
      //
      // The input `area` should be checked against the internal area in
      // order to determine whether we want to clear only part of the widget
      // or all of it. If we want to re-render the whole widget we will just
      // let the argument empty otherwise we need to convert the provided
      // area into a valid local area.

      // Retrieve the internal area.
      utils::Boxf thisSize = LayoutItem::getRenderingArea();

      if (thisSize == area) {
        // Just fill the whole texture.
        getEngine().fillTexture(uuid, getPalette(), nullptr);
      }
      else {
        // We need to convert the input area to a valid coordinate frame
        // which can be interpreted by the engine.
        utils::Boxf converted = convertToEngineFormat(area, thisSize);

        // Perform the repaint.
        getEngine().fillTexture(uuid, getPalette(), &converted);
      }
    }

    inline
    void
    SdlWidget::drawContentPrivate(const utils::Uuid& /*uuid*/,
                                  const utils::Boxf& /*area*/) const
    {
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
      {
        Guard guard(m_childrenLocker);

        // Check for duplicated widget
        if (m_names.find(widget->getName()) != m_names.cend()) {
          error(std::string("Cannot add duplicated widget \"") + widget->getName() + "\"");
        }

        if (m_childrenRepaints.find(widget->getName()) != m_childrenRepaints.cend()) {
          error(std::string("Cannot add duplicated widget \"") + widget->getName() + "\"");
        }
      }

      // Share the data with this widget.
      shareData(widget);

      // Install this object as filter for the child: this will allow to
      // filter out events in case the parent is made invisible.
      widget->installEventFilter(this);

      // Populate internal arrays: first insert the item in the `m_children`
      // array.
      {
        Guard guard(m_childrenLocker);

        m_children.push_back(
          ChildWrapper{
            widget,
            widget->getZOrder()
          }
        );

        // And now rebuilt the `m_names` array after sorting items in ascending
        // z order.
        rebuildZOrdering();
      }
    }

    inline
    void
    SdlWidget::setZOrder(const int order) {
      // Assign the new z order value.
      m_zOrder = order;

      // Notify the parent widget of this modification if any.
      if (hasParent()) {
        postEvent(std::make_shared<core::engine::Event>(core::engine::Event::Type::ZOrderChanged, m_parent));
      }
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
    SdlWidget::clearCachedTexture() {
      if (m_cachedContent.valid()) {
        getEngine().destroyTexture(m_cachedContent);
        m_cachedContent.invalidate();
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
