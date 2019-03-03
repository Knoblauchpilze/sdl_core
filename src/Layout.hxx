#ifndef    LAYOUT_HXX
# define   LAYOUT_HXX

# include "Layout.hh"
# include "SdlException.hh"

namespace sdl {
  namespace core {

    inline
    int
    Layout::addItem(SdlWidget* item,
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
    Layout::removeItem(SdlWidget* item) {
      int index = m_items.size();
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
    sdl::utils::Sizef
    Layout::computeIncompressibleSize(const Direction& direction,
                                      const std::vector<WidgetInfo>& widgets) const
    {
      float flowingSize = 0.0f;
      float perpendicularSize = 0.0f;

      for (unsigned index = 0u ; index < m_items.size() ; ++index) {
        float increment = 0.0f;
        float size = 0.0f;

        if (direction == Direction::Horizontal) {
          // This layout stacks widgets using an horizontal flow:
          // we should add the incompressible size of this widget
          // if it has any in the horizontal direction and retrieve
          // its vertical size if any.
          if (widgets[index].policy.getVerticalPolicy() == sdl::core::SizePolicy::Fixed) {
            size = widgets[index].hint.h();
          }
          increment = widgets[index].hint.w();
        }
        else if (direction == Direction::Vertical) {
          // This layout stacks widgets using a vertical flow:
          // we should add the incompressible size of this widget
          // if it has any in the vertical direction and retrieve
          // its horizontal size if any.
          if (widgets[index].policy.getHorizontalPolicy() == sdl::core::SizePolicy::Fixed) {
            size = widgets[index].hint.w();
          }
          increment = widgets[index].hint.h();
        }
        else {
          throw sdl::core::SdlException(std::string("Unknown direction when updating layout (direction: ") + std::to_string(static_cast<int>(direction)) + ")");
        }

        // Increse the `incompressibleSize` with the provided `size` (which may be
        // 0 if the widget does not have a valid size hint) and performs a comparison
        // of the size of the widget in the other direction (i.e. not in the direction
        // of the flow) against the current maximum and update it if needed.
        flowingSize += increment;
        if (perpendicularSize < size) {
          perpendicularSize = size;
        }
      }

      // Create a valid size based on this layout's direction.
      if (direction == Direction::Horizontal) {
        return sdl::utils::Sizef(flowingSize, perpendicularSize);
      }
      else if (direction == Direction::Vertical) {
        return sdl::utils::Sizef(perpendicularSize, flowingSize);
      }
      else {
        throw sdl::core::SdlException(std::string("Unknown direction when updating layout (direction: ") + std::to_string(static_cast<int>(direction)) + ")");
      }
    }

    inline
    std::pair<bool, bool>
    Layout::canExpand(const WidgetInfo& info,
                      const sdl::utils::Sizef& size) const
    {
      // We need to determine whether the widget can expand in each direction.
      return std::make_pair(
        canExpand(info, Direction::Horizontal, size.w()),
        canExpand(info, Direction::Vertical, size.h())
      );
    }

    inline
    SdlWidget*
    Layout::getContainerOrNull(SdlWidget* item, int* index) const {
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

    inline
    bool
    Layout::canExpand(const WidgetInfo& info,
                      const Direction& direction,
                      const float& desiredSize) const
    {
      // For the rest of this function we will distinguish between the
      // two main cases (e.g. direction set to horizontal or vertical).
      // Basically the same controls are applied, only on different sizes.

      // Retrieve the relevant dimension and size policy.
      bool hintValid = info.hint.isValid();
      float dimension = 0.0f;
      SizePolicy::Policy policy;
      float min = 0.0f;
      float hint = 0.0f;
      float max = 0.0f;

      if (direction == Direction::Horizontal) {
        dimension = info.hint.w();
        policy = info.policy.getHorizontalPolicy();
        min = info.min.w();
        hint = info.hint.w();
        max = info.max.w();
      }
      else if (direction == Direction::Vertical) {
        dimension = info.hint.h();
        policy = info.policy.getVerticalPolicy();
        min = info.min.h();
        hint = info.hint.h();
        max = info.max.h();
      }
      else {
        throw sdl::core::SdlException(std::string("Unknown direction when updating layout (direction: ") + std::to_string(static_cast<int>(direction)) + ")");
      }

      // A widget cannot expand in a given direction if the size policy
      // for this direction is set to `Fixed` and a valid size hint is
      // provided.
      // Unless the desired size corresponds to the provided hint.
      if (hintValid && policy == SizePolicy::Fixed) {
        if (desiredSize == dimension) {
          return true;
        }

        return false;
      }

      // If the `desiredSize` is smaller than the `min`, the widget cannot
      // use the provided size.
      if (desiredSize < min) {
        return false;
      }

      // Conversely if the `desiredSize` is larger than the `max`, the widget
      // cannot use the provided size.
      if (desiredSize > max) {
        return false;
      }

      // If the `desiredSize` is in the range [`min`; `hint`] but the policy
      // is not set to `Shrink`, the provided size is not valid.
      if (desiredSize < hint && !(policy & SizePolicy::Policy::Shrink)) {
        return false;
      }

      // If the `desiredSize` is in the range [`hint`; `max`] but the policy
      // is not set to `Expand`, the provided size is not valid.
      if (desiredSize > hint & !(policy & SizePolicy::Policy::Expand)) {
        return false;
      }

      // Otherwise the desired policy seems valid
      return true;
    }

  }
}

#endif    /* LAYOUT_HXX */
