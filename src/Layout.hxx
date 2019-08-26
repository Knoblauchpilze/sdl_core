#ifndef    LAYOUT_HXX
# define   LAYOUT_HXX

# include "Layout.hh"

namespace sdl {
  namespace core {

    inline
    void
    Layout::addItem(LayoutItem* item,
                    const int& /*index*/)
    {
      // Insert the item.
      addItem(item);
    }

    inline
    void
    Layout::addItem(LayoutItem* item,
                    const unsigned& /*x*/,
                    const unsigned& /*y*/,
                    const unsigned& /*w*/,
                    const unsigned& /*h*/)
    {
      // No specialization at this level, use the base
      // insertion method.
      addItem(item);
    }

    inline
    int
    Layout::removeItem(LayoutItem* item) {
      // Assume the item does not exist (we're a bit pessimistic) and try to find it
      // from the internal table.
      int physID = getIndexOf(item);

      // If we could find the item, remove it.
      if (isValidIndex(physID)) {
        // Convert this index to a logical one.
        const int logicID = getLogicalIDFromPhysicalID(physID);

        // Check whether a logical id could be determined from the input index.
        if (!isValidIndex(logicID)) {
          return -1;
        }

        // Remove the item using the dedicated handler.
        removeItemFromIndex(logicID);

        // Remove the logical id.
        return logicID;
      }

      // Return the index of the removed item (or -1 if it was not found).
      return physID;
    }

    inline
    void
    Layout::removeItemFromIndex(const int item) {
      // Check whether this item can be removed.
      if (!isValidIndex(item)) {
        error(
          std::string("Cannot remove item ") + std::to_string(item),
          std::string("Layout contains only ") + std::to_string(getItemsCount()) + " item(s)"
        );
      }

      // Convert the input logical index into a physical one.
      const int physID = getPhysicalIDFromLogicalID(item);

      if (!isValidIndex(physID)) {
        error(
          std::string("Cannot remove item ") + std::to_string(physID),
          std::string("Layout contains only ") + std::to_string(getItemsCount()) + " item(s)"
        );
      }

      // Remove the item.
      m_items.erase(m_items.cbegin() + physID);

      // Trigger a call to the notifier method.
      const bool rebuild = onIndexRemoved(item, physID);

      // Invalidate the layout if needed.
      if (rebuild) {
        makeGeometryDirty();
      }
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
    bool
    Layout::isNested() const noexcept {
      return m_nesting == Nesting::Deep;
    }

    inline
    void
    Layout::setNesting(const Nesting& nesting) {
      m_nesting = nesting;
    }

    inline
    const utils::Sizef&
    Layout::getMargin() const noexcept {
      return m_margin;
    }

    inline
    void
    Layout::update(const utils::Boxf& window) {
      // Note that calling this method should be handled with care:
      // indeed it triggers an update of the layout and bypass the
      // security put in place in the `LayoutItem` to detect redundant
      // calls to update function.
      updatePrivate(window);
    }

    inline
    void
    Layout::setEventsQueue(engine::EventsQueue* queue) noexcept {
      // Assign the events queue to this object using the base handler.
      LayoutItem::setEventsQueue(queue);

      // Assign the queue to child items if any.
      for (unsigned id = 0u ; id < m_items.size() ; ++id) {
        registerToSameQueue(m_items[id]);
      }
    }

    inline
    bool
    Layout::needsConvert() const noexcept {
      return m_boxesFormat == BoxesFormat::Engine;
    }

    inline
    void
    Layout::setBoxesFormat(const BoxesFormat& format) {
      m_boxesFormat = format;
    }

    inline
    int
    Layout::getIndexOf(LayoutItem* item) const noexcept {
      // If the item is not valid, return -1.
      if (item == nullptr) {
        return -1;
      }

      // Iterate over the internal items to find one equal to the input.
      Items::const_iterator itemToFind = m_items.cbegin();
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
    int
    Layout::getLogicalIDFromPhysicalID(const int physID) const noexcept {
      // Return the input index.
      return physID;
    }

    inline
    int
    Layout::getPhysicalIDFromLogicalID(const int logicID) const noexcept {
      // Return the input index.
      return logicID;
    }

    inline
    bool
    Layout::onIndexRemoved(const int /*logicID*/,
                           const int /*physID*/)
    {
      // Assume a recomputation of the layout is needed.
      return true;
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
