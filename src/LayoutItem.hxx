#ifndef    LAYOUT_ITEM_HXX
# define   LAYOUT_ITEM_HXX

# include "LayoutItem.hh"

namespace sdl {
  namespace core {

    inline
    LayoutItem::LayoutItem(const utils::Sizef& sizeHint):
      m_minSize(),
      m_sizeHint(sizeHint),
      m_maxSize(utils::Sizef::max()),
      m_sizePolicy(),
      m_geometryDirty(true)
    {}

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
    void
    LayoutItem::makeGeometryDirty() {
      // Mark the geometry as dirty.
      m_geometryDirty = true;
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

  }
}

#endif    /* LAYOUT_ITEM_HXX */
