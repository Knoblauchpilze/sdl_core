#ifndef    SDLLAYOUT_HXX
# define   SDLLAYOUT_HXX

# include "SdlLayout.hh"

namespace sdl {
  namespace core {

    inline
    int
    SdlLayout::addItem(std::shared_ptr<SdlWidget> item) {
      if (item != nullptr && getContainerOrNull(item) == nullptr) {
        m_items.push_back(item);
        return m_items.size() - 1;
      }
      return -1;
    }

    inline
    int
    SdlLayout::addItem(std::shared_ptr<SdlWidget> item,
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
    SdlLayout::removeItem(std::shared_ptr<SdlWidget> item) {
      int index = m_items.size();
      getContainerOrNull(item, &index);
      if (index < m_items.size()) {
        m_items.erase(m_items.cbegin() + index);
      }
    }

    inline
    std::shared_ptr<SdlWidget>
    SdlLayout::getContainerOrNull(std::shared_ptr<SdlWidget> item, int* index) const {
      std::vector<std::shared_ptr<SdlWidget>>::const_iterator itemToFind = m_items.cbegin();
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
