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
    utils::Boxf
    LayoutItem::getRenderingArea() const noexcept {
      return m_area;
    }

    inline
    bool
    LayoutItem::isNested() const noexcept {
      return m_nested;
    }

    inline
    void
    LayoutItem::setNested(const bool nested) {
      m_nested = nested;
    }

    inline
    bool
    LayoutItem::needsConvert() const noexcept {
      return m_needsConvert;
    }

    inline
    void
    LayoutItem::setNeedsConvert(const bool needsConvert) {
      m_needsConvert = needsConvert;
    }

    inline
    bool
    LayoutItem::isVirtual() const noexcept {
      return m_virtual;
    }

    inline
    void
    LayoutItem::setVirtual(const bool virtualItem) {
      m_virtual = virtualItem;
    }

    inline
    bool
    LayoutItem::isVisible() const noexcept {
      return m_visible;
    }

    inline
    void
    LayoutItem::setVisible(bool visible) noexcept {
      // Post a show/hide event based on the status of the input `visible` status.
      engine::EventShPtr e;

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
    void
    LayoutItem::makeGeometryDirty() {
      // Mark the geometry as dirty.
      m_geometryDirty = true;

      // Trigger a geometry update event if this item is not part of a
      // virtual hierarchy.
      if (!isVirtual()) {
        postEvent(std::make_shared<engine::Event>(engine::Event::Type::GeometryUpdate));
      }
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
      // Assign the visible status to `false`.
      m_visible = false;

      // Use the base handler to determine the return value.
      return engine::EngineObject::hideEvent(e);
    }

    inline
    bool
    LayoutItem::showEvent(const engine::Event& e) {
      // Assign the visible status to `true`.
      m_visible = true;

      // Use the base handler to determine the return value.
      return engine::EngineObject::showEvent(e);
    }

  }
}

#endif    /* LAYOUT_ITEM_HXX */
