#ifndef    LAYOUT_HXX
# define   LAYOUT_HXX

# include "Layout.hh"

namespace sdl {
  namespace core {

    inline
    int
    Layout::addItem(LayoutItem* item,
                    const unsigned& /*x*/,
                    const unsigned& /*y*/,
                    const unsigned& /*w*/,
                    const unsigned& /*h*/)
    {
      // No specialization at this level.
      return addItem(item);
    }

    inline
    int
    Layout::removeItem(LayoutItem* item) {
      // Assume the item does not exist (we're a bit pessimistic) and try to find it
      // from the internal table.
      int index = getIndexOf(item);

      // If we could find the item, remove it.
      if (isValidIndex(index)) {
        removeItemFromIndex(index);
      }

      // Return the index of the removed item (or -1 if it was not found).
      return index;
    }

    inline
    int
    Layout::getItemsCount() const noexcept {
      return m_items.size();
    }

    inline
    bool
    Layout::empty() const noexcept {
      return getItemsCount() == 0;
    }

    inline
    const utils::Sizef&
    Layout::getMargin() const noexcept {
      return m_margin;
    }

    inline
    void
    Layout::update(const utils::Boxf& window) {
      // Check if this item is a virtual layout, in which case we can proceed
      // to calling the `updatePrivate` method.
      if (isVirtual()) {
        updatePrivate(window);
      }
    }

    inline
    void
    Layout::removeItemFromIndex(int item) {
      // Check whether this item can be removed.
      if (!isValidIndex(item)) {
        error(
          std::string("Cannot remove item ") + std::to_string(item),
          std::string("Layout contains only ") + std::to_string(getItemsCount()) + " item(s)"
        );
      }

      // Remove the item.
      m_items.erase(m_items.cbegin() + item);

      // Invalidate the layout.
      makeGeometryDirty();
    }

    inline
    int
    Layout::getIndexOf(LayoutItem* item) const noexcept {
      // If the item is not valid, return -1.
      if (item == nullptr) {
        return -1;
      }

      // Iterate over the internal items to find one equal to the input.
      std::vector<LayoutItem*>::const_iterator itemToFind = m_items.cbegin();
      int itemID = 0;

      while (itemToFind != m_items.cend() && item != *itemToFind) {
        ++itemToFind;
        ++itemID;
      }

      // Check whether we could find the item.
      if (itemToFind == m_items.cend()) {
        return -1;
      }

      // Return whatever index we reached.
      return itemID;
    }

    inline
    LayoutItem*
    Layout::getItemAt(const int& item) {
      // Check whether the identifier is valid.
      if (!isValidIndex(item)) {
        error(
          std::string("Cannot retrieve item ") + std::to_string(item),
          std::string("Layout contains only ") + std::to_string(getItemsCount()) + " item(s)"
        );
      }

      // Return the item at this location.
      return m_items[item];
    }

    inline
    const LayoutItem*
    Layout::getItemAt(const int& item) const {
      // Check whether the identifier is valid.
      if (!isValidIndex(item)) {
        error(
          std::string("Cannot retrieve item ") + std::to_string(item),
          std::string("Layout contains only ") + std::to_string(getItemsCount()) + " item(s)"
        );
      }

      // Return the item at this location.
      return m_items[item];
    }

    inline
    bool
    Layout::isValidIndex(const int& id) const noexcept {
      return id >= 0 && id < getItemsCount();
    }

    inline
    utils::Sizef
    Layout::computeAvailableSize(const utils::Boxf& totalArea) const noexcept {
      return totalArea.toSize() - 2.0f * m_margin;
    }

    inline
    utils::Sizef
    Layout::computeSpaceAdjustmentNeeded(const utils::Sizef& achieved,
                                         const utils::Sizef& target) const
    {
      return target - achieved;
    }

    inline
    float
    Layout::allocateFairly(const float& space,
                               const unsigned& count) const noexcept
    {
      return space / count;
    }

  }
}

#endif    /* LAYOUT_HXX */
