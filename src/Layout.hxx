#ifndef    LAYOUT_HXX
# define   LAYOUT_HXX

# include "Layout.hh"
# include "LayoutException.hh"

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
    utils::Sizef
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
          throw LayoutException(std::string("Unknown direction when updating layout (direction: ") + std::to_string(static_cast<int>(direction)) + ")");
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
        return utils::Sizef(flowingSize, perpendicularSize);
      }
      else if (direction == Direction::Vertical) {
        return utils::Sizef(perpendicularSize, flowingSize);
      }
      else {
        throw LayoutException(std::string("Unknown direction when updating layout (direction: ") + std::to_string(static_cast<int>(direction)) + ")");
      }
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
    utils::Sizef
    Layout::computeSizeOfWidgets(const Direction& direction,
                                 const std::vector<utils::Boxf>& boxes) const
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
          throw LayoutException(std::string("Unknown direction when updating layout (direction: ") + std::to_string(static_cast<int>(direction)) + ")");
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
        return utils::Sizef(flowingSize, perpendicularSize);
      }
      else if (direction == Direction::Vertical) {
        return utils::Sizef(perpendicularSize, flowingSize);
      }
      else {
        throw LayoutException(std::string("Unknown direction when updating layout (direction: ") + std::to_string(static_cast<int>(direction)) + ")");
      }
    }

    inline
    utils::Sizef
    Layout::computeSizeFromPolicy(const utils::Sizef& desiredSize,
                                  const utils::Boxf& currentSize,
                                  const WidgetInfo& info) const
    {
      // Create the return size and assume the desired size is valid.
      utils::Sizef outputBox(
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
          outputBox.w() = info.hint.w();
          widthDone = true;
        }
      }
      if (info.policy.getVerticalPolicy() == sdl::core::SizePolicy::Fixed) {
        // Two distinct cases:
        // 1) The `hint` is valid, in which case we have to use it.
        // 2) The `hint` is not valid in which case we have to use the `desiredSize`.
        if (info.hint.isValid()) {
          outputBox.h() = info.hint.h();
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
        outputBox.w() = info.min.w();
      }
      if (outputBox.h() < info.min.h()) {
        outputBox.h() = info.min.h();
      }

      if (outputBox.w() > info.max.w()) {
        outputBox.w() = info.max.w();
      }
      if (outputBox.h() > info.max.h()) {
        outputBox.h() = info.max.h();
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
        outputBox.w() = info.hint.w();
      }
      if (outputBox.h() < info.hint.h() && !info.policy.canShrinkVertically()) {
        outputBox.h() = info.hint.h();
      }

      if (outputBox.w() > info.hint.w() && !info.policy.canExtendHorizontally()) {
        outputBox.w() = info.hint.w();
      }
      if (outputBox.h() > info.hint.h() && !info.policy.canExtendVertically()) {
        outputBox.h() = info.hint.h();
      }

      // We can return the computed box.
      return outputBox;
    }

    inline
    sdl::core::SizePolicy
    Layout::shrinkOrGrow(const utils::Sizef& desiredSize,
                         const utils::Sizef& achievedSize,
                         const float& tolerance) const
    {
      // Assume growing in both directions.
      SizePolicy policy(SizePolicy::Fixed, SizePolicy::Fixed);

      // Compare the `achievedSize` to the `desiredSize` and determine the action
      // to apply both horizontally and vertically.
      if (std::abs(desiredSize.w() - achievedSize.w()) < tolerance) {
        // Consider that the `achievedSize` is close enough from the `desiredSize`
        // to keep it.
      }
      else if (desiredSize.w() < achievedSize.w()) {
        std::cout << "[LAY] desired.w() < achieved.w() (" << desiredSize.w() << " < " << achievedSize.w() << "), shrinking" << std::endl;
        policy.setHorizontalPolicy(SizePolicy::Policy::Shrink);
      }
      else if (desiredSize.w() > achievedSize.w()) {
        std::cout << "[LAY] desired.w() > achieved.w() (" << desiredSize.w() << " > " << achievedSize.w() << "), growing" << std::endl;
        policy.setHorizontalPolicy(SizePolicy::Policy::Grow);
      }

      if (std::abs(desiredSize.h() - achievedSize.h()) < tolerance) {
        // Consider that the `achievedSize` is close enough from the `desiredSize`
        // to keep it.
      }
      else if (desiredSize.h() < achievedSize.h()) {
        std::cout << "[LAY] desired.h() < achieved.h() (" << desiredSize.h() << " < " << achievedSize.h() << "), shrinking" << std::endl;
        policy.setVerticalPolicy(SizePolicy::Policy::Shrink);
      }
      else if (desiredSize.h() > achievedSize.h()) {
        std::cout << "[LAY] desired.h() > achieved.h() (" << desiredSize.h() << " > " << achievedSize.h() << "), growing" << std::endl;
        policy.setVerticalPolicy(SizePolicy::Policy::Grow);
      }

      return policy;
    }

    inline
    std::pair<bool, bool>
    Layout::canBeUsedTo(const std::string& name, const WidgetInfo& info,
                        const utils::Boxf& box,
                        const SizePolicy& action) const
    {
      // We want to determine if the widget described by its main
      // information `info` can be used to perform the required
      // operation described in the input `policy` action in the
      // specified direction.
      // We want to return true if the widget can be used to perform
      // at least one of the `action` described by the input
      // argument.
      // The returned value corresponds to a pair describing in
      // its first member whether the widget can be used to perform
      // the `action.getHorizontalPolicy()` and on its second member
      // whether the widget can be used to perform the action
      // described by `action.getVerticalPolicy()`.
      std::pair<bool, bool> usable = std::make_pair(false, false);

      // Let's first handle the case where no valid hint is provided.
      if (!info.hint.isValid()) {
        // The result of this function is solely based on the current
        // size of the widget versus the `min` and `max` size.
        // Also, we need to consider both directions: as soon as a
        // valid `action` can be performed, we need to return but
        // failure to perform one action should not stop the process.

        // Check for shrinking.
        if (action.canShrinkHorizontally() && box.w() > info.min.w()) {
          std::cout << "[WIG] " << name << " can be used to h shrink (" << box.w() << " > " << info.min.w() << ")" << std::endl;
          usable.first = true;
        }
        if (action.canShrinkVertically() && box.h() > info.min.h()) {
          std::cout << "[WIG] " << name << " can be used to v shrink (" << box.h() << " > " << info.min.h() << ")" << std::endl;
          usable.second = true;
        }

        // Check for growing.
        if (action.canExtendHorizontally() && box.w() < info.max.w()) {
          std::cout << "[WIG] " << name << " can be used to h grow (" << box.w() << " < " << info.max.w() << ")" << std::endl;
          usable.first = true;
        }
        if (action.canExtendVertically() && box.h() < info.max.h()) {
          std::cout << "[WIG] " << name << " can be used to v grow (" << box.h() << " < " << info.max.h() << ")" << std::endl;
          usable.second = true;
        }
      }

      // If an hint is provided, we also need to compare the box to the
      // hint. Indeed in case the policy is set to `Shrink` for example,
      // if an hint is provided if effectively replace the `max` size
      // for all intent and purposes.
      // Respectively if the policy is set to `Grow` and an hint is
      // provided, the `hint` replaces the `min` size.

      if (info.hint.isValid()) {
        // Check for shrinking.
        if (action.canShrinkHorizontally()) {
          // The action requires to shrink: the widget can do that if
          // its policy is set to `Shrink` and the current size is
          // larger than the `min` or if the policy is NOT set with
          // the `Shrink` flag, the `hint` replaces the value.
          if (info.policy.canShrinkHorizontally() && box.w() > info.min.w()) {
            std::cout << "[WIG] " << name << " can be used to h shrink (" << box.w() << " > " << info.min.w() << ")" << std::endl;
            usable.first = true;
          }
          if (!info.policy.canShrinkHorizontally() && box.w() > info.hint.w()) {
            std::cout << "[WIG] " << name << " cannot be used to h shrink but (" << box.w() << " > " << info.hint.w() << ")" << std::endl;
            usable.first = true;
          }
        }
        if (action.canShrinkVertically()) {
          // The action requires to shrink: the widget can do that if
          // its policy is set to `Shrink` and the current size is
          // larger than the `min` or if the policy is NOT set with
          // the `Shrink` flag, the `hint` replaces the value.
          if (info.policy.canShrinkVertically() && box.h() > info.min.h()) {
            std::cout << "[WIG] " << name << " can be used to v shrink (" << box.h() << " > " << info.min.h() << ")" << std::endl;
            usable.second = true;
          }
          if (!info.policy.canShrinkVertically() && box.h() > info.hint.h()) {
            std::cout << "[WIG] " << name << " cannot be used to v shrink but (" << box.h() << " > " << info.hint.h() << ")" << std::endl;
            usable.second = true;
          }
        }

        // Check for growing.
        if (action.canExtendHorizontally()) {
          // The action requires to shrink: the widget can do that if
          // its policy is set to `Shrink` and the current size is
          // larger than the `min` or if the policy is NOT set with
          // the `Shrink` flag, the `hint` replaces the value.
          if (info.policy.canExtendHorizontally() && box.w() < info.max.w()) {
            std::cout << "[WIG] " << name << " can be used to h grow (" << box.w() << " < " << info.max.w() << ")" << std::endl;
            usable.first = true;
          }
          if (!info.policy.canExtendHorizontally() && box.w() < info.hint.w()) {
            std::cout << "[WIG] " << name << " cannot be used to v grow but (" << box.w() << " < " << info.hint.w() << ")" << std::endl;
            usable.first = true;
          }
        }
        if (action.canExtendVertically()) {
          // The action requires to shrink: the widget can do that if
          // its policy is set to `Shrink` and the current size is
          // larger than the `min` or if the policy is NOT set with
          // the `Shrink` flag, the `hint` replaces the value.
          if (info.policy.canExtendVertically() && box.h() < info.max.h()) {
            std::cout << "[WIG] " << name << " can be used to v grow (" << box.h() << " < " << info.max.h() << ")" << std::endl;
            usable.second = true;
          }
          if (!info.policy.canExtendVertically() && box.h() < info.hint.h()) {
            std::cout << "[WIG] " << name << " cannot be used to h grow but (" << box.h() << " < " << info.hint.h() << ")" << std::endl;
            usable.second = true;
          }
        }
      }

      // If at this point the `usable` pair is still uniformly `false`,
      // we can deduce that:
      // 1) We don't have a valid hint but the widget can neither shrink
      //    nor grow to match the desired size.
      // 2) We have a valid hint but the widget can neither shrink nor
      //    grow to match the desired size.
      // It seems like we have a fixed size widget: thus there's no way
      // the widget can be used to perform the desired `action`.
      // In any case the default values are correct so we can just return
      // the status.
      return usable;
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
