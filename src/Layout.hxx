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

        // Increase the `flowingSize` with the provided `increment` (which may be
        // 0 if the widget does not have a valid size hint) and perform a comparison
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
      if (desiredSize > hint & !(policy & SizePolicy::Policy::Grow) && !(policy & SizePolicy::Policy::Expand)) {
        return false;
      }

      // Otherwise the desired policy seems valid
      return true;
    }

    inline
    sdl::utils::Sizef
    Layout::computeSizeOfWidgets(const Direction& direction,
                                 const std::vector<sdl::utils::Boxf>& boxes) const
    {
      float flowingSize = 0.0f;
      float perpendicularSize = 0.0f;

      for (unsigned index = 0u ; index < boxes.size() ; ++index) {
        float increment = 0.0f;
        float size = 0.0f;

        if (direction == Direction::Horizontal) {
          // The `increment` is given by the width of the box while the
          // `size` is given by its height.
          size = boxes[index].h();
          increment = boxes[index].w();
        }
        else if (direction == Direction::Vertical) {
          // The `increment` is given by the height of the box while the
          // `size` is given by its width.
          size = boxes[index].w();
          increment = boxes[index].h();
        }
        else {
          throw sdl::core::SdlException(std::string("Unknown direction when updating layout (direction: ") + std::to_string(static_cast<int>(direction)) + ")");
        }

        // Increase the `flowingSize` with the provided `increment` and
        // perform a comparison of the size of the widget in the other
        // direction (i.e. not in the direction of the flow) against the
        // current maximum and update it if needed.
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
    sdl::core::SizePolicy::Policy
    Layout::shrinkOrGrow(const Direction& direction,
                         const sdl::utils::Sizef& desiredSize,
                         const sdl::utils::Sizef& achievedSize) const
    {
      // According to the direction, we have to check the correct dimension
      // to determine whether we should shrink or grow.
      SizePolicy::Policy policy;

      if (direction == Direction::Horizontal) {
        // Assume we need to grow.
        policy = SizePolicy::Policy::Grow;
        if (desiredSize.w() < achievedSize.w()) {
          policy = SizePolicy::Policy::Shrink;
        }
      }
      else if (direction == Direction::Vertical) {
        // Assume we need to grow.
        policy = SizePolicy::Policy::Grow;
        if (desiredSize.h() < achievedSize.h()) {
          policy = SizePolicy::Policy::Shrink;
        }
      }
      else {
        throw sdl::core::SdlException(std::string("Unknown direction when updating layout (direction: ") + std::to_string(static_cast<int>(direction)) + ")");
      }

      return policy;
    }

    inline
    bool
    Layout::canBeUsedTo(const WidgetInfo& info,
                        const sdl::utils::Boxf& box,
                        const SizePolicy::Policy& action,
                        const Direction& direction) const
    {
      // We want to determine if the widget described by its main
      // information `info` can be used to perform the required
      // operation described in the input `policy` action in the
      // specified direction.

      // First we need to retrieve the relevant operation based on
      // the input direction.
      float size = 0.0f;
      SizePolicy::Policy policy;
      float min = 0.0f;
      float hint = 0.0f;
      float max = 0.0f;

      if (direction == Direction::Horizontal) {
        size = box.w();
        policy = info.policy.getHorizontalPolicy();
        min = info.min.w();
        hint = info.hint.w();
        max = info.max.w();
      }
      else if (direction == Direction::Vertical) {
        size = box.h();
        policy = info.policy.getVerticalPolicy();
        min = info.min.h();
        hint = info.hint.h();
        max = info.max.h();
      }
      else {
        throw sdl::core::SdlException(std::string("Unknown direction when updating layout (direction: ") + std::to_string(static_cast<int>(direction)) + ")");
      }

      // If the widget has a fixed size policy and a valid hint, there's
      // no way it can be used to do anything.
      if (policy == SizePolicy::Fixed && info.hint.isValid()) {
        return false;
      }

      // Once we eliminate the fixed size policy case, there's no real
      // problem with doing anything as long as the provided action does
      // not make the widget bypass its bounds.

      // Let's first handle the case where no valid hint is provided.
      if (!info.hint.isValid()) {
        // The widget can srhrink as long as its size is not equal
        // to its lower bound.
        if (action == SizePolicy::Policy::Shrink) {
          return size > min;
        }

        // The widget can expand as long as its size is not equal
        // to its upper bound.
        if (action == SizePolicy::Policy::Grow) {
          return size < max;
        }
      }

      // If the size hint is valid, this offset the bound check and we
      // also need to check for the internal size policy of the widget.

      // The widget can shrink if either the policy is set to `Shrink`
      // or if the `size` is larger than the hint.
      if (action == SizePolicy::Policy::Shrink) {
        return
          (size > min && policy & SizePolicy::Policy::Shrink) ||
          (size > hint)
        ;
      }

      // The widget can expand if either the policy is set to `Expand`
      // or if the `size` is smaller than the hint.
      if (action == SizePolicy::Policy::Shrink) {
        return
          (size < max && (policy & SizePolicy::Policy::Grow || policy & SizePolicy::Policy::Expand)) ||
          (size > hint)
        ;
      }
    }

    inline
    sdl::utils::Sizef
    Layout::computeSpaceAdjustmentNeeded(const sdl::utils::Sizef& achieved,
                                         const sdl::utils::Sizef& target) const
    {
      return sdl::utils::Sizef(target.w() - achieved.w(), target.h() - achieved.h());
    }

  }
}

#endif    /* LAYOUT_HXX */
