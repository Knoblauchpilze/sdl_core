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
    LayoutItem::isRootItem() const noexcept {
      return m_rootItem;
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
      return m_geometryDirty;
    }

    inline
    void
    LayoutItem::geometryRecomputed() noexcept {
      m_geometryDirty = false;
    }

    inline
    void
    LayoutItem::setRoot(const bool isRoot) {
      m_rootItem = isRoot;
    }

    inline
    void
    LayoutItem::updatePrivate(const utils::Boxf& window) {
      if (m_area != window) {
        m_area = window;
      }
    }

  }
}

#endif    /* LAYOUT_ITEM_HXX */
