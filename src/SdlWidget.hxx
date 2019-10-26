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
    SdlWidget::getRenderingArea() const noexcept {
      // Lock this widget.
      Guard guard(m_contentLocker);

      // Return the value provided by the base handler.
      return LayoutItem::getRenderingArea();
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
      return utils::Boxf(globalOffset, thisBox.w(), thisBox.h());
    }

    inline
    std::string
    SdlWidget::getZOrderString(const LayoutItem* stop) const noexcept {
      // Create the output string.
      std::string orders;

      // Gather the information from the parent if needed (i.e. if
      // the parent exists and if we're not the `stop` widget).
      if (hasParent() && this != stop) {
        orders = m_parent->getZOrderString(stop);
      }

      // Append the `z` order from this widget as computed by the base
      // class.
      orders += LayoutItem::getZOrderString();

      // Return the built-in string.
      return orders;
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
    bool
    SdlWidget::filterEvent(engine::EngineObject* watched,
                           engine::EventShPtr e)
    {
      // Use the base method to filter the events as usual: this will provide a
      // first basis to detect whether event can be filtered at this level.
      if (LayoutItem::filterEvent(watched, e)) {
        // The event is filtered
        return true;
      }

      // Check whether the parent filters it, in which case we
      // should filter it too.
      if (hasParent() && m_parent->filterEvent(watched, e)) {
        return true;
      }

      // Finally if the widget is managed, we should ask the layout if
      // the event is filtered. This allow some sort of communication
      // between siblings widget.
      if (isManaged() && getManager()->filterEvent(watched, e)) {
        return true;
      }

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
    bool
    SdlWidget::showEvent(const engine::Event& e) {
      // Use the base handler to perform needed internal updates.
      const bool toReturn = LayoutItem::showEvent(e);

      // Trigger a repaint event if the widget is set to visible.
      if (isVisible()) {
        makeContentDirty();
      }

      // Return the value provided by the base handler.
      return toReturn;
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

      // Delete the widget to release the memory.
      delete widget;

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
    SdlWidget::isAncestor(const SdlWidget* widget) const noexcept {
      // This widget is an ancestor of the input `widget` if the input
      // `widget` is a descendant of `this` widget.
      return widget != nullptr && widget->isDescendant(this);
    }

    inline
    bool
    SdlWidget::isDescendant(const SdlWidget* widget) const noexcept {
      // This widget is a descendant of the input `widget` if the input
      // `widget` is the parent of `this` widget or if it is an ancestor
      // of its parent.
      return widget != nullptr && hasParent() && (m_parent == widget || widget->isAncestor(m_parent));
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
    SdlWidget::getChildAs(const std::string& name) const {
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
    SdlWidget::getChildOrNull(const std::string& name) const {
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
      // This kind of event is generated whenever the mouse just entered the
      // current widget. The main goal of this method is to update the mouse
      // position (which is now by definition inside the widget) and to trigger
      // a focus event with the corresponding reason.
      // The actual update of the widget's content based on the focus is left
      // to be handled in the focus event.

      // Post a focus event with the specified reason: redraw of the widget's
      // content is left to be processed there.
      postEvent(engine::FocusEvent::createFocusInEvent(engine::FocusEvent::Reason::HoverFocus, true));

      // Use base handler to determine whether the event was recognized.
      return LayoutItem::enterEvent(e);
    }

    inline
    bool
    SdlWidget::hideEvent(const engine::Event& e) {
      // Handling a hide event as a widget comes with a double responsability.
      // Indeed the hide event can either originate from `this` widget or be
      // sent by a child of ours. In the first case we of course need to handle
      // the necessary operations to hide ourselves, but also notify our parent
      // if any so that it can repaint itself and erase all traces of our data.
      // If the event comes from one of our children, we need to trigger said
      // repaint operations.

      // Assume the event is recognized.
      bool toReturn = true;

      // Check whether the hide event concerns `this` widget.
      if (isEmitter(e)) {
        // Trigger the process to hide `this` widget.
        toReturn = LayoutItem::hideEvent(e);

        // Also notify the parent from this hide operation.
        engine::EventShPtr he = std::make_shared<engine::Event>(engine::Event::Type::Hide);
        engine::EngineObject* o = nullptr;

        if (hasParent()) {
          he->setReceiver(m_parent);
          o = m_parent;
        }
        else if (isManaged()) {
          he->setReceiver(getManager());
          o = getManager();
        }

        if (o == nullptr) {
          log("Do not post hide event to parent, no need to do so", utils::Level::Info);
        }

        if (o != nullptr) {
          postEvent(he, false, true);
        }
      }

      // In case the event comes from one of the child of this
      // widget we need to schedule a repaint event for the area
      // occupied by the widget. Otherwise we would get remains
      // of the widget which has just been hidden displayed which
      // would not be cool.

      // Determine whether the event comes from on of the child
      // of `this` widget.
      if (e.isSpontaneous() || !hasChild(e.getEmitter()->getName())) {
        // The hide event should probably not have been sent to
        // `this` widget. Do nothing more.
        return toReturn;
      }

      // Query the rendering area for this widget and request a
      // repaint operation on it to erase remains of the widget
      // that has just been hidden.
      SdlWidget* child = getChildAs<SdlWidget>(e.getEmitter()->getName());

      // TODO: In the case of a combobox, we are getting the rendering
      // area of a children which probably does not have a valid position
      // as its position was computed when the size of the combobox was
      // dropped which leads to inconsistent repaints.
      log("Handling hide for " + child->getName() + " with area " + child->getRenderingArea().toString() + " (global: " + mapToGlobal(child->getRenderingArea(), true).toString() + ")");

      engine::PaintEventShPtr pe = std::make_shared<engine::PaintEvent>(mapToGlobal(child->getRenderingArea(), true));
      postEvent(pe, true, true);

      // Transmit the return value.
      return toReturn;
    }

    inline
    bool
    SdlWidget::mouseButtonReleaseEvent(const engine::MouseEvent& e) {
      // Mouse events are only transmitted to this widget when the mouse is
      // inside the widget and if no other child block the view.
      // In the case no child widget blocks the mouse we need to check if
      // this type of interaction is handled by `this` widget.
      // This is done automatically by the `FocusIn` events so let's just
      // create one.

      // If the mouse is blocked by a child, do nothing.
      if (isBlockedByChild(e.getMousePosition())) {
        // Return early using the base handler return value.
        return LayoutItem::mouseButtonReleaseEvent(e);
      }

      // The mouse is not blocked by any child: produce a focus event.
      postEvent(engine::FocusEvent::createFocusInEvent(engine::FocusEvent::Reason::MouseFocus, true));

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
      // as it's inside the widget and not blocked by any child.
      // child.
      // Basically we only want to trigger a enter event if this
      // is the first time we receive such an event (which can be
      // checked by verifying that he mouse is still considered
      // outside of this widget).
      if (!isMouseInside()) {
        postEvent(std::make_shared<engine::EnterEvent>(e.getMousePosition()));
      }

      // Use base handler to determine whether the event was recognized.
      return LayoutItem::mouseMoveEvent(e);
    }

    inline
    utils::Uuid
    SdlWidget::createContentPrivate(const engine::Palette::ColorRole& role) const {
      // Create the texture using the engine. The dmensions are retrieved from the
      // internal area.
      utils::Boxf area = LayoutItem::getRenderingArea();
      utils::Uuid uuid = getEngine().createTexture(area.toSize(), role);

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
      utils::Boxf thisBox = LayoutItem::getRenderingArea();

      if (thisBox == area) {
        // Just fill the whole texture.
        getEngine().fillTexture(uuid, getPalette(), nullptr);
      }
      else {
        // Compute the intersection between the area and the area described by
        // this widget: if no intersection are found we can skip everything
        // and otherwise we only want to repaint the needed area.
        // Note that we want to retrieve the size of this area and not the
        // box expressed in the parent's coordinate frame.
        utils::Boxf thisArea = thisBox.toOrigin();
        utils::Boxf inter = thisArea.intersect(area);

        if (!inter.valid()) {
          // Nothing to repaint, the area to clear does not intersect the area
          // assigned to this widget.
          return;
        }

        // We need to convert the input area to a valid coordinate frame
        // which can be interpreted by the engine.
        utils::Boxf converted = convertToEngineFormat(inter, thisBox);

        // Perform the repaint.
        getEngine().fillTexture(uuid, getPalette(), &converted);
      }
    }

    inline
    void
    SdlWidget::drawContentPrivate(const utils::Uuid& /*uuid*/,
                                  const utils::Boxf& /*area*/)
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
    SdlWidget::updateStateFromFocus(const engine::FocusEvent& e) {
      // We need to update the widget's content to match the new focus state.
      // We will use the dedicated focus state and determine whether we need
      // to update `this` widget's content afterwards.
      // Depending on the source of the event we need to update the internal
      // focus state as well: the external focus state (provided by the base
      // `getFocusState` method) is always updated because it reflects the
      // state of the tree defined by `this` widget to the outside world but
      // the internal state is only modified when a focus is specifically
      // directed towards `this` widget.
      const bool gainedFocus =
        e.getType() == engine::Event::Type::FocusIn ||
        e.getType() == engine::Event::Type::GainFocus
      ;

      // Always update the external focus state.
      FocusState& state = getFocusState();
      if (gainedFocus) {
        state.handleFocusIn(e.getReason());
      }
      else {
        state.handleFocusOut(e.getReason());
      }

      // The external focus state has been updated. Now we want to trigger the
      // internal `stateUpdatedFromFocus` method as prescribed by the interface
      // of the method so that inheriting classes can perform some additional
      // modifications to the representation of this widget.
      // This can either happen if the focus reason is effecitvely directed to
      // `this` widget OR if the update describes a focus lost. Indeed a focus
      // lost event is always by definition produced by another item in the tree.
      // Finally we also only trigger the `stateUpdatedFromFocus` in case the
      // focus reason can be handled.
      if (!canHandleFocusReason(e.getReason())) {
        return;
      }

      // We now want to handle the modification of the internal state as we are
      // certain that the focus reason can be handled. We have actually several
      // cases which can happen here.
      // Basically what is important is to determine whether the focus event can
      // have some influence on the visual representation of `this` widget. The
      // simple part is when the focus event actually comes from `this` widget.
      // In this case we are pretty sure that the visual representation will be
      // updated.
      // Problem comes in when the source is not `this` widget. One important
      // thing to note is that the actual type of input events can only be one
      // of the following:
      // - `FocusIn`
      // - `FocusOut`
      // - `GainFocus`
      // The `GainFocus` along with the `FocusOut` should be transformed before
      // being processed. While the `FocusOut` event is only received when it
      // is produced by `this` widget, the `GainFocus` event on the other hand
      // only comes from other widgets (and mostly child on top of that). In
      // both cases it means that we should lose the focus, otherwise we would
      // not be transmitted the event in the first place. In addition, we can
      // consider that the internal state will handle the redundancy in case
      // the widget is not focused.
      // Basically imagine the situation where one of our child is actually in
      // focus and gets deactivated: we will (as its parent) receive a focus out
      // event, but if everything went smoothy we should not have our internal
      // state in a value which allows for deactivation: indeed we never received
      // a `FocusIn` event, only a `GainFocus` one.
      bool updated = false;

      // Handle only `FocusIn` and `FocusOut` events.
      std::string save = m_internalFocusState.toString();
      if (e.getType() == engine::Event::Type::FocusIn) {
        // A `FocusIn` events transformed into a `FocusOut` event if `this` widget
        // is not the emitter.
        if (isEmitter(e)) {
          updated = m_internalFocusState.handleFocusIn(e.getReason());
        }
        else {
          updated = m_internalFocusState.handleFocusOut(e.getReason());
        }
      }
      else if (e.getType() == engine::Event::Type::FocusOut ||
               e.getType() == engine::Event::Type::GainFocus)
      {
        updated = m_internalFocusState.handleFocusOut(e.getReason());
      }

      // If the internal state has been updated, trigger a calls to the interface
      // method which is the basic guarantee of this method.
      if (updated) {
        stateUpdatedFromFocus(m_internalFocusState, gainedFocus);
      }
    }

    inline
    void
    SdlWidget::stateUpdatedFromFocus(const FocusState& state,
                                     const bool /*gainedFocus*/)
    {
      // The default implementation specifies that the content's texture role
      // should be updated to reflect the internal focus state of the widget.
      // We also need to trigger a repaint event in case the content's role is
      // modified. This can only occur if the texture representing the content
      // is valid, obviously.

      // If the content is not valid, nothing can be done.
      if (!m_content.valid()) {
        log("Trashing texture role update because content is not valid", utils::Level::Warning);
        return;
      }

      log("Texture role is now " + std::to_string(static_cast<int>(state.getColorRole())) + " " + getPalette().getColorForRole(state.getColorRole()).toString());

      // Assign the texture role based on the color associated to the input
      // state: there's a handler which conveniently provide the color role
      // associated to its current value.
      getEngine().setTextureRole(m_content, state.getColorRole());

      // Post a repaint event.
      requestRepaint();
    }

    inline
    bool
    SdlWidget::canCauseKeyboardFocusChange(const engine::FocusEvent::Reason& reason) const noexcept {
      // As a base definition the `Tab` and `Click` focus reasons can trigger a keyboard focus change.
      return
        reason == engine::FocusEvent::Reason::MouseFocus ||
        reason == engine::FocusEvent::Reason::TabFocus ||
        reason == engine::FocusEvent::Reason::BacktabFocus
      ;
    }

    inline
    engine::Palette::ColorRole
    SdlWidget::clearTexture() {
      // Assume default color role.
      engine::Palette::ColorRole role = engine::Palette::ColorRole::Background;

      // Destroy the content if any.
      if (m_content.valid()) {
        // Update the color role.
        role = getEngine().getTextureRole(m_content);

        // Destroy the texture.
        getEngine().destroyTexture(m_content);
        m_content.invalidate();
      }

      // Return the color role.
      return role;
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
