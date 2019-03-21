#ifndef    LAYOUT_HXX
# define   LAYOUT_HXX

# include "Layout.hh"

namespace sdl {
  namespace core {

    inline
    int
    Layout::addItem(SdlWidget* item,
                    const unsigned& /*x*/,
                    const unsigned& /*y*/,
                    const unsigned& /*w*/,
                    const unsigned& /*h*/)
    {
      // No specialization at this level.
      return addItem(item);
    }

    inline
    void
    Layout::removeItem(SdlWidget* item) {
      std::size_t index = m_items.size();
      getContainerOrNull(item, &index);
      if (index < m_items.size()) {
        m_items.erase(m_items.cbegin() + index);
        invalidate();
      }
    }

    inline
    unsigned
    Layout::getItemsCount() const noexcept {
      return m_items.size();
    }

    inline
    void
    Layout::invalidate() noexcept {
      m_dirty = true;
    }

    inline
    SdlWidget*
    Layout::getContainerOrNull(SdlWidget* item, std::size_t* index) const {
      std::vector<SdlWidget*>::const_iterator itemToFind = m_items.cbegin();
      std::size_t itemId = 0;
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

    inline
    utils::Sizef
    Layout::computeSpaceAdjustmentNeeded(const utils::Sizef& achieved,
                                         const utils::Sizef& target) const
    {
      return utils::Sizef(target.w() - achieved.w(), target.h() - achieved.h());
    }

  }
}

#endif    /* LAYOUT_HXX */
