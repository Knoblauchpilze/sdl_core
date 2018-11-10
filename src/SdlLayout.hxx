#ifndef    SDLLAYOUT_HXX
# define   SDLLAYOUT_HXX

# include "SdlLayout.hh"

namespace sdl {
  namespace core {

    inline
    int
    SdlLayout::addItem(SdlWidget* item) {
      if (item != nullptr && getContainerOrNull(item) == nullptr) {
        m_items.push_back(item);
        makeDirty();
        return m_items.size() - 1;
      }
      return -1;
    }

    inline
    int
    SdlLayout::addItem(SdlWidget* item,
                    const unsigned& x,
                    const unsigned& y,
                    const unsigned& w,
                    const unsigned& h)
    {
      // No specialization at this level.
      return addItem(item);
    }

    inline
    void
    SdlLayout::removeItem(SdlWidget* item) {
      int index = m_items.size();
      getContainerOrNull(item, &index);
      if (index < m_items.size()) {
        m_items.erase(m_items.cbegin() + index);
        makeDirty();
      }
    }

    inline
    unsigned
    SdlLayout::getItemsCount() const noexcept {
      return m_items.size();
    }

    inline
    void
    SdlLayout::makeDirty() noexcept {
      m_dirty = true;
    }


    inline
    SdlWidget*
    SdlLayout::getContainerOrNull(SdlWidget* item, int* index) const {
      std::vector<SdlWidget*>::const_iterator itemToFind = m_items.cbegin();
      int itemId = 0;
      while (itemToFind != m_items.cend() && item != *itemToFind) {
        ++itemToFind;
        ++itemId;
      }

      if (index != nullptr) {
        *index = itemId;
      }
      if (itemToFind == m_items.cend()) {
        return nullptr;
      }

      return *itemToFind;
    }

  }
}

#endif    /* SDLLAYOUT_HXX */
