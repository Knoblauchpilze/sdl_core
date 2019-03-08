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
    sdl::utils::Sizef
    Layout::computeSizeFromPolicy(const sdl::utils::Sizef& desiredSize,
                                  const sdl::utils::Boxf& currentSize,
                                  const WidgetInfo& info) const
    {
      // Create the return size and assume the desired size is valid.
      sdl::utils::Sizef outputBox(
        currentSize.w() + desiredSize.w(),
        currentSize.h() + desiredSize.h()
      );

      bool widthDone = false;
      bool heightDone = false;

      // Check the policy for fixed size. If the policy is fixed, we should assign
      // the `hint` size whatever the input `desiredSize`. Except of course if the
      // `hint` is not a valid size, in which case we can use the `desiredSize`.
      if (info.policy.getHorizontalPolicy() == sdl::core::SizePolicy::Fixed) {
        // Two distinct cases:
        // 1) The `hint` is valid, in which case we have to use it.
        // 2) The `hint` is not valid in which case we have to use the `desiredSize`.
        if (info.hint.isValid()) {
          outputBox.setWidth(info.hint.w());
          widthDone = true;
        }
      }
      if (info.policy.getVerticalPolicy() == sdl::core::SizePolicy::Fixed) {
        // Two distinct cases:
        // 1) The `hint` is valid, in which case we have to use it.
        // 2) The `hint` is not valid in which case we have to use the `desiredSize`.
        if (info.hint.isValid()) {
          outputBox.setHeight(info.hint.h());
          heightDone = true;
        }
      }

      // Check whether we should continue further.
      if (widthDone && heightDone) {
        return outputBox;
      }

      // At least one of the dimension is not set to fixed, so we have to check for
      // min and max sizes.
      if (outputBox.w() < info.min.w()) {
        outputBox.setWidth(info.min.w());
      }
      if (outputBox.h() < info.min.h()) {
        outputBox.setHeight(info.min.h());
      }

      if (outputBox.w() > info.max.w()) {
        outputBox.setWidth(info.max.w());
      }
      if (outputBox.h() > info.max.h()) {
        outputBox.setHeight(info.max.h());
      }

      // The last thing to check concerns the size policy. For example if the `desiredSize`
      // is larger than the provided hint, even though the `desiredSize` is smaller than the
      // `maxSize`, if the policy is not set to `Grow`, we should still use the `hint` size.
      // Same goes for the case where the `desiredSize` lies in the interval [`minSize`; `hint`]
      // and the policy is not set to `Shrink`: the `hint` should be used.
      // If course all this is only relevant if the hint is valid, otherwise we can use the
      // `desiredSize`.
      if (!info.hint.isValid()) {
        // Nothing more to do, the `desiredSize` can be used once clamped using the `minSize`
        // and `maxSize`.
        return outputBox;
      }

      // Check shrinking policy.
      if (outputBox.w() < info.hint.w() && !info.policy.canShrinkHorizontally()) {
        outputBox.setWidth(info.hint.w());
      }
      if (outputBox.h() < info.hint.h() && !info.policy.canShrinkVertically()) {
        outputBox.setHeight(info.hint.h());
      }

      if (outputBox.w() > info.hint.w() && !info.policy.canExtendHorizontally()) {
        outputBox.setWidth(info.hint.w());
      }
      if (outputBox.h() > info.hint.h() && !info.policy.canExtendVertically()) {
        outputBox.setHeight(info.hint.h());
      }

      // We can return the computed box.
      return outputBox;
    }

    inline
    sdl::core::SizePolicy
    Layout::shrinkOrGrow(const sdl::utils::Sizef& desiredSize,
                         const sdl::utils::Sizef& achievedSize) const
    {
      // Assume growing in both directions.
      SizePolicy policy(SizePolicy::Minimum, SizePolicy::Minimum);

      // Compare the `achievedSize` to the `desiredSize` and determine the action
      // to apply both horizontally and vertically.
      if (desiredSize.w() < achievedSize.w()) {
        policy.setHorizontalPolicy(SizePolicy::Policy::Shrink);
      }

      if (desiredSize.h() < achievedSize.h()) {
        policy.setVerticalPolicy(SizePolicy::Policy::Shrink);
      }

      return policy;
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
                        const SizePolicy& action) const
    {
      // We want to determine if the widget described by its main
      // information `info` can be used to perform the required
      // operation described in the input `policy` action in the
      // specified direction.
      // We want to return true if the widget can be used to perform
      // at least one of the `action` described by the input
      // argument.

      // Let's first handle the case where no valid hint is provided.
      if (!info.hint.isValid()) {
        // The result of this function is solely based on the current
        // size of the widget versus the `min` and `max` size.
        // Also, we need to consider both directions: as soon as a
        // valid `action` can be performed, we need to return but
        // failure to perform one action should not stop the process.

        // Check for shrinking.
        if (action.canShrinkHorizontally() && box.w() > info.min.w()) {
          return true;
        }
        if (action.canShrinkVertically() && box.h() > info.min.h()) {
          return true;
        }

        // Check for growing.
        if (action.canExtendHorizontally() && box.w() < info.max.w()) {
          return true;
        }
        if (action.canExtendVertically() && box.h() < info.max.h()) {
          return true;
        }
      }

      // If an hint is provided, we also need to compare the box to the
      // hint. Indeed in case the policy is set to `Shrink` for example,
      // if an hint is provided if effectively replace the `max` size
      // for all intent and purposes.
      // Respectively if the policy is set to `Grow` and an hint is
      // provided, the `hint` replaces the `min` size.

      // Check for shrinking.
      if (action.canShrinkHorizontally()) {
        // The action requires to shrink: the widget can do that if
        // its policy is set to `Shrink` and the current size is
        // larger than the `min` or if the policy is NOT set with
        // the `Shrink` flag, the `hint` replaces the value.
        if (info.policy.canShrinkHorizontally() && box.w() > info.min.w()) {
          return true;
        }
        if (!info.policy.canShrinkHorizontally() && box.w() > info.hint.w()) {
          return true;
        }
      }
      if (action.canShrinkVertically()) {
        // The action requires to shrink: the widget can do that if
        // its policy is set to `Shrink` and the current size is
        // larger than the `min` or if the policy is NOT set with
        // the `Shrink` flag, the `hint` replaces the value.
        if (info.policy.canShrinkVertically() && box.h() > info.min.h()) {
          return true;
        }
        if (!info.policy.canShrinkVertically() && box.h() > info.hint.h()) {
          return true;
        }
      }

      // Check for growing.
      if (action.canExtendHorizontally()) {
        // The action requires to shrink: the widget can do that if
        // its policy is set to `Shrink` and the current size is
        // larger than the `min` or if the policy is NOT set with
        // the `Shrink` flag, the `hint` replaces the value.
        if (info.policy.canExtendHorizontally() && box.w() < info.max.w()) {
          return true;
        }
        if (!info.policy.canExtendHorizontally() && box.w() < info.hint.w()) {
          return true;
        }
      }
      if (action.canExtendVertically()) {
        // The action requires to shrink: the widget can do that if
        // its policy is set to `Shrink` and the current size is
        // larger than the `min` or if the policy is NOT set with
        // the `Shrink` flag, the `hint` replaces the value.
        if (info.policy.canExtendVertically() && box.h() < info.max.h()) {
          return true;
        }
        if (!info.policy.canExtendVertically() && box.h() < info.hint.h()) {
          return true;
        }
      }

      // At this point, we know that:
      // 1) We don't have a valid hint but the widget can neither shrink
      //    nor grow to match the desired size.
      // 2) We have a valid hint but the widget can neither shrink nor
      //    grow to match the desired size.
      // It seems like we have a fixed size widget: thus there's no way
      // the widget can be used to perform the desired `action`.
      return false;
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

      // Let's first handle the case where no valid hint is provided.
      if (!info.hint.isValid()) {
        // The result of this function is solely based on the current
        // size of the widget versus the `min` and `max` size.
        // Also, we need to consider both directions: as soon as a
        // valid `action` can be performed, we need to return but
        // failure to perform one action should not stop the process.

        // Check for shrinking.
        if (action == SizePolicy::Policy::Shrink) {
          return size > min;
        }

        // Check for growing.
        if (action == SizePolicy::Policy::Grow || action == SizePolicy::Policy::Expand) {
          return size < max;
        }
      }

      // If an hint is provided, we also need to compare the box to the
      // hint. Indeed in case the policy is set to `Shrink` for example,
      // if an hint is provided if effectively replace the `max` size
      // for all intent and purposes.
      // Respectively if the policy is set to `Grow` and an hint is
      // provided, the `hint` replaces the `min` size.

      // Check for shrinking.
      if (action == SizePolicy::Policy::Shrink) {
        // The action requires to shrink: the widget can do that if
        // its policy is set to `Shrink` and the current size is
        // larger than the `min` or if the policy is NOT set with
        // the `Shrink` flag, the `hint` replaces the value.
        return
          (size > min && policy & SizePolicy::Policy::Shrink) ||
          (size > hint)
        ;
      }

      // Check for growing.
      if (action == SizePolicy::Policy::Grow || action == SizePolicy::Policy::Expand) {
        // The action requires to shrink: the widget can do that if
        // its policy is set to `Shrink` and the current size is
        // larger than the `min` or if the policy is NOT set with
        // the `Shrink` flag, the `hint` replaces the value.
        return
          (size < max && (policy & SizePolicy::Policy::Grow || policy & SizePolicy::Policy::Expand)) ||
          (size < hint)
        ;
      }

      // At this point, we know that:
      // 1) We don't have a valid hint but the widget can neither shrink
      //    nor grow to match the desired size.
      // 2) We have a valid hint but the widget can neither shrink nor
      //    grow to match the desired size.
      // It seems like we have a fixed size widget: thus there's no way
      // the widget can be used to perform the desired `action`.
      return false;
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
