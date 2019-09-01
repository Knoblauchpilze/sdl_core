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
      m_minSize = size;
      makeGeometryDirty();
    }

    inline
    utils::Sizef
    LayoutItem::getSizeHint() const noexcept {
      return m_sizeHint;
    }

    inline
    void
    LayoutItem::setSizeHint(const utils::Sizef& hint) noexcept {
      m_sizeHint = hint;
      makeGeometryDirty();
    }

    inline
    utils::Sizef
    LayoutItem::getMaxSize() const noexcept {
      return m_maxSize;
    }

    inline
    void
    LayoutItem::setMaxSize(const utils::Sizef& size) noexcept {
      m_maxSize = size;
      makeGeometryDirty();
    }

    inline
    SizePolicy
    LayoutItem::getSizePolicy() const noexcept {
      return m_sizePolicy;
    }

    inline
    void
    LayoutItem::setSizePolicy(const SizePolicy& policy) noexcept {
      m_sizePolicy = policy;
      makeGeometryDirty();
    }

    inline
    FocusPolicy
    LayoutItem::getFocusPolicy() const noexcept {
      return m_focusPolicy;
    }

    inline
    void
    LayoutItem::setFocusPolicy(const FocusPolicy& policy) noexcept {
      m_focusPolicy = policy;
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
      return m_state.hasFocus();
    }

    inline
    FocusState&
    LayoutItem::getFocusState() noexcept {
      return m_state;
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
      std::lock_guard<std::mutex> guard(m_visibleLocker);
      return m_visible;
    }

    inline
    void
    LayoutItem::setVisible(bool visible) noexcept {
      // Post a show/hide event based on the status of the input `visible` status.
      engine::EventShPtr e;

      // Issue an event based on the current status.
      if (visible) {
        e = std::make_shared<engine::Event>(engine::Event::Type::Show, this);
      }
      else {
        e = std::make_shared<engine::Event>(engine::Event::Type::Hide, this);
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
    LayoutItem*
    LayoutItem::getManager() const noexcept {
      return m_manager;
    }

    inline
    bool
    LayoutItem::canHandleFocusReason(const engine::FocusEvent::Reason& reason) const noexcept {
      switch (reason) {
        case engine::FocusEvent::Reason::HoverFocus:
          return getFocusPolicy().canGrabHoverFocus();
        case engine::FocusEvent::Reason::MouseFocus:
          return getFocusPolicy().canGrabClickFocus();
        case engine::FocusEvent::Reason::TabFocus:
        case engine::FocusEvent::Reason::BacktabFocus:
          return getFocusPolicy().canGrabTabFocus();
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
    LayoutItem::updatePrivate(const utils::Boxf& window) {
      if (m_area != window) {
        m_area = window;
      }
    }

    inline
    bool
    LayoutItem::hideEvent(const engine::Event& e) {
      // We only want to do that in case the event originates from this item.
      if (!isEmitter(e)) {
        return engine::EngineObject::hideEvent(e);
      }

      // Assign the corresponding visible status. 
      {
        std::lock_guard<std::mutex> guard(m_visibleLocker);
        m_visible = false;
      }

      // Deactivate the events for this item: the layout
      // items only handle focus and show events when
      // hidden.
      std::unordered_set<engine::Event::Type> all = engine::Event::getAllEvents();
      all.erase(engine::Event::Type::Show);
      all.erase(engine::Event::Type::FocusIn);
      all.erase(engine::Event::Type::FocusOut);
      all.erase(engine::Event::Type::GainFocus);
      all.erase(engine::Event::Type::LostFocus);

      disableEventsProcessing(all);

      // Use the base handler to determine the return value.
      return engine::EngineObject::hideEvent(e);
    }

    inline
    bool
    LayoutItem::showEvent(const engine::Event& e) {
      // We only want to do that in case the event originates from this item.
      if (!isEmitter(e)) {
        return engine::EngineObject::showEvent(e);
      }

      // Assign the corresponding visible status.
      {
        std::lock_guard<std::mutex> guard(m_visibleLocker);
        m_visible = true;
      }
      // TODO: Handle the modification of the way the Show/Hide events work.

      // Reactivate event handling.
      activateEventsProcessing();

      // Use the base handler to determine the return value.
      return engine::EngineObject::showEvent(e);
    }

  }
}

#endif    /* LAYOUT_ITEM_HXX */
