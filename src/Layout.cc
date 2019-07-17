
# include "Layout.hh"
# include "SdlWidget.hh"

namespace sdl {
  namespace core {

    Layout::Layout(const std::string& name,
                   SdlWidget* widget,
                   const float& margin):
      LayoutItem(name, utils::Sizef(), true, false),
      m_items(),
      m_margin(utils::Sizef(margin, margin))
    {
      // Assign the events queue from the container if needed.
      if (widget != nullptr) {
        widget->registerToSameQueue(this);
      }

      setService(std::string("layout"));
    }

    Layout::~Layout() {
      // Do not assign a null layout to the managed container:
      // we assume that the container is managing this layout so
      // if we reach this point it means that the container purposefully
      // deleted the layout and thus we can only cry because we're
      // getting replaced.
    }

    void
    Layout::updatePrivate(const utils::Boxf& window) {
      // Use base handler to perform needed modifications.
      LayoutItem::updatePrivate(window);

      // And if some items are managed by this layout.
      if (empty()) {
        return;
      }

      // Proceed by activating the internal handler.
      computeGeometry(window);
    }

    bool
    Layout::repaintEvent(const engine::PaintEvent& e) {
      // The input paint event describes some areas to update. We need to propagate the paint
      // event to the children which intersect the areas to repaint. We should also avoid sending
      // the paint event to the child which may have sent this paint event. This can be done by
      // checking the emitter of the event: if it corredponds to any of the child, we don't send
      // the event to it when we encounter it.
      // In order to not flood the children with unneeded areas, we create a new paint event for
      // each one which contains only the relevant areas.
      const std::vector<utils::Boxf>& regions = e.getUpdateRegions();

      log(
        "Handling repaint for event containing " + std::to_string(regions.size()) + " region(s) to update (source: " +
        (e.getEmitter() == nullptr ? "null" : e.getEmitter()->getName()) + ")",
        utils::Level::Error
      );

      // Traverse the internal array of children.
      for (Items::const_iterator child = m_items.cbegin() ;
           child != m_items.cend() ;
           ++child)
      {
        // Discard this child if it is the emitter of the event.
        if ((*child) == e.getEmitter()) {
          log("Ignoring child " + (*child)->getName() + " which is the source of the paint event");
          continue;
        }

        // Also disacrd the child if it is not visible.
        if (!(*child)->isVisible()) {
          log("Ignoring child " + (*child)->getName() + " which is not visible");
          continue;
        }

        // Create a paint event for this children.
        engine::PaintEventShPtr pe = std::make_shared<engine::PaintEvent>(*child);
        pe->setEmitter(this);

        // Select only update areas which spans at least a portion
        // of this child's area.
        for (int id = 0 ; id < static_cast<int>(regions.size()) ; ++id) {
          if (regions[id].intersects((*child)->getDrawingArea(), true)) {
            log("Area " + regions[id].toString() + " intersects area of " + (*child)->getName() + " (area: " + (*child)->getDrawingArea().toString() + ")");
            pe->addUpdateRegion(regions[id]);
          }
        }

        // Send this event if it contains at least an update area.
        if (pe->hasUpdateRegions()) {
          (*child)->postEvent(pe, false, false);
        }
      }

      // Use the base method to handle the return value.
      return LayoutItem::repaintEvent(e);
    }

    int
    Layout::addItem(LayoutItem* item) {
      // Check for valid items.
      if (item == nullptr) {
        // Invalid item to add.
        return -1;
      }

      // Check for duplicated items.
      if (isValidIndex(getIndexOf(item))) {
        error(
          std::string("Cannot add item \"") + item->getName() + "\" to layout",
          std::string("Item already exist")
        );
      }

      // Insert the item into the layout.
      m_items.push_back(item);

      // Set this item as `managed` by this layout.
      item->setManager(this);

      // Compute the physical id of this item.
      const int physID = m_items.size() - 1;

      // Invalidate the layout.
      makeGeometryDirty();

      // Retrieve the logical id for this item.
      return physID;
    }

    int
    Layout::getIndexOf(const std::string& name) const noexcept {
      // Traverse the internal array of items and try to find one matching
      // the input name. If no such element can be found, return a negative
      // value to indicate to the caller that we could not find the provided
      // name.

      // Traverse the internal array until we find a item which name matches
      // the input.
      unsigned id = 0u;
      while (id < m_items.size() && (m_items[id] == nullptr || m_items[id]->getName() != name)) {
        ++id;
      }

      // Check whether we could find an item with the specified name.
      if (id >= m_items.size()) {
        // We could not find an item with the specified name.
        return -1;
      }

      // Return the index of this item.
      return static_cast<int>(id);
    }

    void
    Layout::assignRenderingAreas(const std::vector<utils::Boxf>& boxes,
                                 const utils::Boxf& window)
    {
      // Assign the rendering area to items.
      for (unsigned index = 0u; index < boxes.size() ; ++index) {
        // The origin of the coordinate frame of the rendering areas is defined as
        // the center of the area available in the parent widget.
        // When exporting from `computeGeometry`, the input `boxes` are defined such
        // that the `x` and `y` coordinate corresponds to the position of the top
        // left corner of the box. But we want to provide a centered box to assign
        // to the widget. Thus we need to convert from this top left representation
        // into a centered one.
        // Also, the input `boxes` are computed relatively to the `window` which
        // means that the implicit origin for the top left coordinate corresponds
        // to the half-dimensions of the `window`. So we need to factor that out
        // when computing the coordinates of the center of each box.

        // We provide a way to bypass this mechanism by defining the layout as
        // nested: in this case we assume that the layout is part of a hierarchy of
        // component in which case the areas will already be provided by some other
        // layout, and thus already converted.

        utils::Boxf converted = boxes[index];

        if (needsConvert()) {
          // So first compute the coordinates of the center of the box, by using the
          // position of the top left corner and adding the half-dimensions.
          const float xCenter = boxes[index].x() + boxes[index].w() / 2.0f - window.w() / 2.0f;
          const float yCenter = boxes[index].y() + boxes[index].h() / 2.0f - window.h() / 2.0f;

          // Now we need to convert into a relative coordinate frame based on the
          // `window` argument: basically the center of the `window` box (i.e. the
          // point of coordinates `[window.x(), window.y()]` as the `window` is
          // provided using a *centered* represetnation) will be transformed to
          // `[0; 0]` and we must adapt the computed `[xCenter; yCenter]` to the
          // same convention.

          // Compute the offset needed between the desired center and the center of the
          // input `window`. Note that as the `y` axis is inverted, the expressions for
          // `x` and `y` differ slightly.
          float offsetX = xCenter;
          float offsetY = -yCenter;

          // Account for nested layouts where we need to use the window position to
          // offse the children boxes.
          if (isNested()) {
            offsetX += window.x();
            offsetY += window.y();
          }

          // Create a converted box.
          converted = utils::Boxf(offsetX, offsetY, boxes[index].w(), boxes[index].h());
        }

        // log("Area for " + m_items[index]->getName() + " is " + converted.toString() + " from " + boxes[index].toString() + " (window: " + window.toString() + ")");
        log("Area for " + m_items[index]->getName() + " is " + converted.toString());

        postEvent(std::make_shared<engine::ResizeEvent>(converted, m_items[index]->getRenderingArea(), m_items[index]));
      }
    }

    void
    Layout::assignVisibilityStatus(const std::vector<bool>& visible) {
      // Assign the rendering area to items.
      for (unsigned index = 0u; index < visible.size() ; ++index) {
        m_items[index]->setVisible(visible[index]);
      }
    }

    SizePolicy
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
        log(std::string("achieved.w() > desired.w() (") + std::to_string(achievedSize.w()) + " > " + std::to_string(desiredSize.w()) + "), shrinking", utils::Level::Notice);
        policy.setHorizontalPolicy(SizePolicy::Policy::Shrink);
      }
      else if (desiredSize.w() > achievedSize.w()) {
        log(std::string("achieved.w() < desired.w() (") + std::to_string(achievedSize.w()) + " < " + std::to_string(desiredSize.w()) + "), growing", utils::Level::Notice);
        policy.setHorizontalPolicy(SizePolicy::Policy::Grow);
      }

      if (std::abs(desiredSize.h() - achievedSize.h()) < tolerance) {
        // Consider that the `achievedSize` is close enough from the `desiredSize`
        // to keep it.
      }
      else if (desiredSize.h() < achievedSize.h()) {
        log(std::string("achieved.h() > desired.h() (") + std::to_string(achievedSize.h()) + " > " + std::to_string(desiredSize.h()) + "), shrinking", utils::Level::Notice);
        policy.setVerticalPolicy(SizePolicy::Policy::Shrink);
      }
      else if (desiredSize.h() > achievedSize.h()) {
        log(std::string("achieved.h() < desired.h() (") + std::to_string(achievedSize.h()) + " < " + std::to_string(desiredSize.h()) + "), growing", utils::Level::Notice);
        policy.setVerticalPolicy(SizePolicy::Policy::Grow);
      }

      return policy;
    }

    std::vector<Layout::WidgetInfo>
    Layout::computeItemsInfo() const noexcept {
      // Create the return value.
      std::vector<Layout::WidgetInfo> info(m_items.size());

      // Fill each item's info.
      for (unsigned index = 0u ; index < m_items.size() ; ++index) {
        info[index] = {
          m_items[index]->getSizePolicy(),
          m_items[index]->getMinSize(),
          m_items[index]->getSizeHint(),
          m_items[index]->getMaxSize(),
          m_items[index]->getRenderingArea(),
          m_items[index]->isVisible()
        };
      }

      return info;
    }

    float
    Layout::computeWidthFromPolicy(const utils::Boxf& currentSize,
                                   const float& delta,
                                   const WidgetInfo& info) const
    {
      // Create the return width and assume the desired width is valid.
      float output = currentSize.w() + delta;

      bool widthDone = false;

      // Check the policy for fixed size. If the policy is fixed, we should assign
      // the `hint` size whatever the input `delta`. Except of course if the
      // `hint` is not a valid size, in which case we can use the `sizeDelta`.
      if (info.policy.getHorizontalPolicy() == SizePolicy::Fixed) {
        // Two distinct cases:
        // 1) The `hint` is valid, in which case we have to use it.
        // 2) The `hint` is not valid in which case we have to use the `sizeDelta`.
        if (info.hint.isValid()) {
          output = info.hint.w();
          widthDone = true;
        }
      }

      // Check whether we should continue further.
      if (widthDone) {
        return output;
      }

      // The width is not set to fixed so we have to check for min and max sizes.
      if (output < info.min.w()) {
        output = info.min.w();
      }
      if (output > info.max.w()) {
        output = info.max.w();
      }

      // The last thing to check concerns the size policy. For example if the `output`
      // is larger than the provided hint, even though the `output` is smaller than the
      // `maxSize`, if the policy is not set to `Grow`, we should still use the `hint` size.
      // Same goes for the case where the `output` lies in the interval [`minSize`; `hint`]
      // and the policy is not set to `Shrink`: the `hint` should be used.
      // If course all this is only relevant if the hint is valid, otherwise we can use the
      // `output`.
      if (!info.hint.isValid()) {
        // Nothing more to do, the `output` can be used once clamped using the `minSize`
        // and `maxSize`.
        return output;
      }

      // Check shrinking policy.
      if (output < info.hint.w() && !info.policy.canShrinkHorizontally()) {
        output = info.hint.w();
      }
      if (output > info.hint.w() && !info.policy.canExtendHorizontally()) {
        output = info.hint.w();
      }

      // We can return the computed width.
      return output;
    }

    float
    Layout::computeHeightFromPolicy(const utils::Boxf& currentSize,
                                    const float& delta,
                                    const WidgetInfo& info) const
    {
      // Create the return height and assume the desired height is valid.
      float output = currentSize.h() + delta;

      bool heightDone = false;

      // Check the policy for fixed size. If the policy is fixed, we should assign
      // the `hint` size whatever the input `delta`. Except of course if the
      // `hint` is not a valid size, in which case we can use the `sizeDelta`.
      if (info.policy.getVerticalPolicy() == SizePolicy::Fixed) {
        // Two distinct cases:
        // 1) The `hint` is valid, in which case we have to use it.
        // 2) The `hint` is not valid in which case we have to use the `sizeDelta`.
        if (info.hint.isValid()) {
          output = info.hint.h();
          heightDone = true;
        }
      }

      // Check whether we should continue further.
      if (heightDone) {
        return output;
      }

      // The height is not set to fixed so we have to check for min and max sizes.
      if (output < info.min.h()) {
        output = info.min.h();
      }
      if (output > info.max.h()) {
        output = info.max.h();
      }

      // The last thing to check concerns the size policy. For example if the `output`
      // is larger than the provided hint, even though the `output` is smaller than the
      // `maxSize`, if the policy is not set to `Grow`, we should still use the `hint` size.
      // Same goes for the case where the `output` lies in the interval [`minSize`; `hint`]
      // and the policy is not set to `Shrink`: the `hint` should be used.
      // If course all this is only relevant if the hint is valid, otherwise we can use the
      // `output`.
      if (!info.hint.isValid()) {
        // Nothing more to do, the `output` can be used once clamped using the `minSize`
        // and `maxSize`.
        return output;
      }

      // Check shrinking policy.
      if (output < info.hint.h() && !info.policy.canShrinkVertically()) {
        output = info.hint.h();
      }
      if (output > info.hint.h() && !info.policy.canExtendVertically()) {
        output = info.hint.h();
      }

      // We can return the computed height.
      return output;
    }

    utils::Sizef
    Layout::computeSizeFromPolicy(const utils::Boxf& currentSize,
                                  const utils::Sizef& sizeDelta,
                                  const WidgetInfo& info) const
    {
      // Use the dedicated handler to clamp each dimension of the size.
      return utils::Sizef(
        computeWidthFromPolicy(currentSize, sizeDelta.w(), info),
        computeHeightFromPolicy(currentSize, sizeDelta.h(), info)
      );
    }

    std::pair<bool, bool>
    Layout::canBeUsedTo(const std::string& /*name*/, const WidgetInfo& info,
                        const utils::Boxf& box,
                        const SizePolicy& action) const
    {
      // We want to determine if the item described by its main
      // information `info` can be used to perform the required
      // operation described in the input `policy` action in the
      // specified direction.
      // We want to return true if the item can be used to perform
      // at least one of the `action` described by the input
      // argument.
      // The returned value corresponds to a pair describing in
      // its first member whether the item can be used to perform
      // the `action.getHorizontalPolicy()` and on its second member
      // whether the item can be used to perform the action
      // described by `action.getVerticalPolicy()`.
      std::pair<bool, bool> usable = std::make_pair(false, false);

      // Let's first handle the case where no valid hint is provided.
      if (!info.hint.isValid()) {
        // The result of this function is solely based on the current
        // size of the item versus the `min` and `max` size.
        // Also, we need to consider both directions: as soon as a
        // valid `action` can be performed, we need to return but
        // failure to perform one action should not stop the process.

        // Check for shrinking.
        if (action.canShrinkHorizontally() && box.w() > info.min.w()) {
          // std::cout << "[WIG] " << name << " can be used to h shrink (" << box.w() << " > " << info.min.w() << ")" << std::endl;
          usable.first = true;
        }
        if (action.canShrinkVertically() && box.h() > info.min.h()) {
          // std::cout << "[WIG] " << name << " can be used to v shrink (" << box.h() << " > " << info.min.h() << ")" << std::endl;
          usable.second = true;
        }

        // Check for growing.
        if (action.canExtendHorizontally() && box.w() < info.max.w()) {
          // std::cout << "[WIG] " << name << " can be used to h grow (" << box.w() << " < " << info.max.w() << ")" << std::endl;
          usable.first = true;
        }
        if (action.canExtendVertically() && box.h() < info.max.h()) {
          // std::cout << "[WIG] " << name << " can be used to v grow (" << box.h() << " < " << info.max.h() << ")" << std::endl;
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
          // The action requires to shrink: the item can do that if
          // its policy is set to `Shrink` and the current size is
          // larger than the `min` or if the policy is NOT set with
          // the `Shrink` flag, the `hint` replaces the value.
          if (info.policy.canShrinkHorizontally() && box.w() > info.min.w()) {
            // std::cout << "[WIG] " << name << " can be used to h shrink (" << box.w() << " > " << info.min.w() << ")" << std::endl;
            usable.first = true;
          }
          if (!info.policy.canShrinkHorizontally() && box.w() > info.hint.w()) {
            // std::cout << "[WIG] " << name << " cannot be used to h shrink but (" << box.w() << " > " << info.hint.w() << ")" << std::endl;
            usable.first = true;
          }
        }
        if (action.canShrinkVertically()) {
          // The action requires to shrink: the item can do that if
          // its policy is set to `Shrink` and the current size is
          // larger than the `min` or if the policy is NOT set with
          // the `Shrink` flag, the `hint` replaces the value.
          if (info.policy.canShrinkVertically() && box.h() > info.min.h()) {
            // std::cout << "[WIG] " << name << " can be used to v shrink (" << box.h() << " > " << info.min.h() << ")" << std::endl;
            usable.second = true;
          }
          if (!info.policy.canShrinkVertically() && box.h() > info.hint.h()) {
            // std::cout << "[WIG] " << name << " cannot be used to v shrink but (" << box.h() << " > " << info.hint.h() << ")" << std::endl;
            usable.second = true;
          }
        }

        // Check for growing.
        if (action.canExtendHorizontally()) {
          // The action requires to shrink: the item can do that if
          // its policy is set to `Shrink` and the current size is
          // larger than the `min` or if the policy is NOT set with
          // the `Shrink` flag, the `hint` replaces the value.
          if (info.policy.canExtendHorizontally() && box.w() < info.max.w()) {
            // std::cout << "[WIG] " << name << " can be used to h grow (" << box.w() << " < " << info.max.w() << ")" << std::endl;
            usable.first = true;
          }
          if (!info.policy.canExtendHorizontally() && box.w() < info.hint.w()) {
            // std::cout << "[WIG] " << name << " cannot be used to v grow but (" << box.w() << " < " << info.hint.w() << ")" << std::endl;
            usable.first = true;
          }
        }
        if (action.canExtendVertically()) {
          // The action requires to shrink: the item can do that if
          // its policy is set to `Shrink` and the current size is
          // larger than the `min` or if the policy is NOT set with
          // the `Shrink` flag, the `hint` replaces the value.
          if (info.policy.canExtendVertically() && box.h() < info.max.h()) {
            // std::cout << "[WIG] " << name << " can be used to v grow (" << box.h() << " < " << info.max.h() << ")" << std::endl;
            usable.second = true;
          }
          if (!info.policy.canExtendVertically() && box.h() < info.hint.h()) {
            // std::cout << "[WIG] " << name << " cannot be used to h grow but (" << box.h() << " < " << info.hint.h() << ")" << std::endl;
            usable.second = true;
          }
        }
      }

      // If at this point the `usable` pair is still uniformly `false`,
      // we can deduce that:
      // 1) We don't have a valid hint but the item can neither shrink
      //    nor grow to match the desired size.
      // 2) We have a valid hint but the item can neither shrink nor
      //    grow to match the desired size.
      // It seems like we have a fixed size item: thus there's no way
      // the item can be used to perform the desired `action`.
      // In any case the default values are correct so we can just return
      // the status.
      return usable;
    }

  }
}
