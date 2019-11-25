#ifndef    LAYOUT_ITEM_HXX
# define   LAYOUT_ITEM_HXX

# include "LayoutItem.hh"

namespace sdl {
  namespace core {

    inline
    LayoutItem::~LayoutItem() {}

    inline
    utils::Sizef
    LayoutItem::getMinSize() const noexcept {
      return m_minSize;
    }

    inline
    void
    LayoutItem::setMinSize(const utils::Sizef& size) noexcept {
      // Update only if both sizes are different.
      if (size != m_minSize) {
        m_minSize = size;
        makeGeometryDirty();
      }
    }

    inline
    utils::Sizef
    LayoutItem::getSizeHint() const noexcept {
      return m_sizeHint;
    }

    inline
    void
    LayoutItem::setSizeHint(const utils::Sizef& hint) noexcept {
      // Update only if both sizes are different.
      if (hint != m_sizeHint) {
        m_sizeHint = hint;
        makeGeometryDirty();
      }
    }

    inline
    utils::Sizef
    LayoutItem::getMaxSize() const noexcept {
      return m_maxSize;
    }

    inline
    void
    LayoutItem::setMaxSize(const utils::Sizef& size) noexcept {
      // Update only if both sizes are different.
      if (size != m_maxSize) {
        m_maxSize = size;
        makeGeometryDirty();
      }
    }

    inline
    SizePolicy
    LayoutItem::getSizePolicy() const noexcept {
      return m_sizePolicy;
    }

    inline
    void
    LayoutItem::setSizePolicy(const SizePolicy& policy) noexcept {
      if (policy != m_sizePolicy) {
        m_sizePolicy = policy;
        makeGeometryDirty();
      }
    }

    inline
    FocusPolicy
    LayoutItem::getFocusPolicy() const noexcept {
      return m_focusPolicy;
    }

    inline
    void
    LayoutItem::setFocusPolicy(const FocusPolicy& policy) noexcept {
      if (policy != m_focusPolicy) {
        m_focusPolicy = policy;
      }
    }

    inline
    utils::Boxf
    LayoutItem::getRenderingArea() const noexcept {
      return m_area;
    }

    inline
    utils::Boxf
    LayoutItem::getDrawingArea() const noexcept {
      return m_area;
    }

    inline
    bool
    LayoutItem::hasFocus() const noexcept {
      return m_focusState.hasFocus();
    }

    inline
    FocusState&
    LayoutItem::getFocusState() noexcept {
      return m_focusState;
    }

    inline
    int
    LayoutItem::getZOrder() const noexcept {
      return m_zOrder;
    }

    inline
    std::string
    LayoutItem::getZOrderString(const LayoutItem* /*stop*/) const noexcept {
      return std::to_string(getZOrder());
    }

    inline
    void
    LayoutItem::setZOrder(int order) {
      // Assign the new z order value.
      m_zOrder = order;

      // Create a new event of the corresponding type.
      postEvent(std::make_shared<engine::Event>(engine::Event::Type::ZOrderChanged));
    }

    inline
    bool
    LayoutItem::hasKeyboardFocus() const noexcept {
      return m_keyboardFocus;
    }

    inline
    bool
    LayoutItem::isManaged() const noexcept {
      return m_manager != nullptr;
    }

    inline
    void
    LayoutItem::setManager(LayoutItem* item) noexcept {
      m_manager = item;
    }

    inline
    bool
    LayoutItem::isVisible() const noexcept {
      Guard guard(m_visibleLocker);
      return m_visible;
    }

    inline
    void
    LayoutItem::setVisible(bool visible) noexcept {
      // Post a show/hide event based on the status of the input `visible` status.
      engine::EventShPtr e;

      // Issue an event based on the current status.
      if (visible) {
        e = std::make_shared<engine::Event>(engine::Event::Type::Show);
      }
      else {
        e = std::make_shared<engine::HideEvent>(getDrawingArea());
      }

      // Post this event.
      postEvent(e);
    }

    inline
    void
    LayoutItem::invalidate() {
      makeGeometryDirty();
    }

    inline
    bool
    LayoutItem::filterEvent(engine::EngineObject* watched,
                            engine::EventShPtr e)
    {
      // Handle mouse events: if the mouse event should be filtered we don't apply
      // other filters. Otherwise it continues to cascade through the filters.
      engine::MouseEventShPtr me = std::dynamic_pointer_cast<engine::MouseEvent>(e);
      if (me != nullptr && filterMouseEvents(watched, me)) {
        return true;
      }

      // Handle keyboard events: same principle as for mouse events.
      engine::KeyEventShPtr ke = std::dynamic_pointer_cast<engine::KeyEvent>(e);
      if (ke != nullptr && filterKeyboardEvents(watched, ke)) {
        return true;
      }

      // Apply filtering on drag and drop events.
      engine::DropEventShPtr de = std::dynamic_pointer_cast<engine::DropEvent>(e);
      if (de != nullptr && filterDragAndDropEvents(watched, de)) {
        return true;
      }

      // All the attempts at filtering the input event failed: use the base class
      // method to provide a return value.
      return engine::EngineObject::filterEvent(watched, e);
    }

    inline
    LayoutItem*
    LayoutItem::getManager() const noexcept {
      return m_manager;
    }

    inline
    bool
    LayoutItem::staysActiveWhileDisabled(const engine::Event::Type& type) const noexcept {
      // Focus events stays active while disabled, along with any event allowed in the base
      // class method.
      return
        EngineObject::staysActiveWhileDisabled(type) ||
        type == engine::Event::Type::FocusIn ||
        type == engine::Event::Type::FocusOut ||
        type == engine::Event::Type::GainFocus ||
        type == engine::Event::Type::LostFocus
      ;
    }

    inline
    bool
    LayoutItem::staysInactiveWhileEnabled(const engine::Event::Type& type) const noexcept {
      // Window event stays disabled while the item is active, along with any event disabled
      // by the base class method.
      return
        EngineObject::staysInactiveWhileEnabled(type) ||
        type == engine::Event::Type::WindowEnter ||
        type == engine::Event::Type::WindowLeave ||
        type == engine::Event::Type::WindowResize ||
        type == engine::Event::Type::Quit
      ;
    }

    inline
    bool
    LayoutItem::canHandleFocusReason(const engine::FocusEvent::Reason& reason) const noexcept {
      switch (reason) {
        case engine::FocusEvent::Reason::HoverFocus:
          return canGrabHoverFocus(getFocusPolicy());
        case engine::FocusEvent::Reason::MouseFocus:
          return canGrabClickFocus(getFocusPolicy());
        case engine::FocusEvent::Reason::TabFocus:
        case engine::FocusEvent::Reason::BacktabFocus:
          return canGrabTabFocus(getFocusPolicy());
        default:
          return false;
      }
    }

    inline
    void
    LayoutItem::makeGeometryDirty() {
      // Mark the geometry as dirty.
      m_geometryDirty = true;

      // Trigger a geometry update event.
      postEvent(std::make_shared<engine::Event>(engine::Event::Type::GeometryUpdate));
    }

    inline
    bool
    LayoutItem::hasGeometryChanged() const noexcept {
      return m_geometryDirty && isVisible();
    }

    inline
    void
    LayoutItem::geometryRecomputed() noexcept {
      m_geometryDirty = false;
    }

    inline
    void
    LayoutItem::updatePrivate(const utils::Boxf& /*window*/) {
      // Empty implementation.
    }

    inline
    bool
    LayoutItem::filterKeyboardEvents(const engine::EngineObject* /*watched*/,
                                     const engine::KeyEventShPtr /*e*/) const noexcept
    {
      // Empty implementation.
      return false;
    }

    inline
    bool
    LayoutItem::hideEvent(const engine::HideEvent& e) {
      // We only want to do that in case the event originates from this item.
      if (!isEmitter(e)) {
        return engine::EngineObject::hideEvent(e);
      }

      // Assign the corresponding visible status.
      bool changed = false;
      {
        Guard guard(m_visibleLocker);
        changed = (m_visible != false);
        m_visible = false;
      }

      // Deactivate the events for this item only if we actually changed the
      // internal status of the item.
      if (changed) {
        disableEventsProcessing();
      }

      // Use the base handler to determine the return value.
      return engine::EngineObject::hideEvent(e);
    }

    inline
    bool
    LayoutItem::keyboardGrabbedEvent(const engine::Event& e) {
      log("Item now has keyboard focus", utils::Level::Notice);

      // Update the keyboard focus status.
      m_keyboardFocus = true;

      // Use the base handler to provide a return value.
      return engine::EngineObject::keyboardGrabbedEvent(e);
    }

    inline
    bool
    LayoutItem::keyboardReleasedEvent(const engine::Event& e) {
      log("Item has lost keyboard focus", utils::Level::Notice);

      // Update the keyboard focus status.
      m_keyboardFocus = false;

      // Use the base handler to provide a return value.
      return engine::EngineObject::keyboardReleasedEvent(e);
    }

    inline
    bool
    LayoutItem::showEvent(const engine::Event& e) {
      // We only want to do that in case the event originates from this item.
      if (!isEmitter(e)) {
        return engine::EngineObject::showEvent(e);
      }

      // Assign the corresponding visible status.
      bool changed = false;
      {
        Guard guard(m_visibleLocker);
        changed = (m_visible != true);
        m_visible = true;
      }

      // Activate the events for this item only if we actually changed the
      // internal status of the item.
      if (changed) {
        activateEventsProcessing();
      }

      // Use the base handler to determine the return value.
      return engine::EngineObject::showEvent(e);
    }

  }
}

#endif    /* LAYOUT_ITEM_HXX */
