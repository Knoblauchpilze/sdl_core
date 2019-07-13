
# include "SdlWidget.hh"
# include <core_utils/CoreWrapper.hh>

namespace sdl {
  namespace core {

    SdlWidget::SdlWidget(const std::string& name,
                         const utils::Sizef& sizeHint,
                         SdlWidget* parent,
                         const engine::Color& color):
      LayoutItem(name, sizeHint, false, false),

      m_names(),
      m_children(),
      m_layout(),
      m_zOrder(0),
      m_palette(engine::Palette::fromButtonColor(color)),
      m_engine(nullptr),

      m_parent(nullptr),

      m_contentDirty(true),

      m_content(),
      m_drawingLocker(),
      m_repaintOperation(nullptr),
      m_refreshOperation(nullptr),
      m_cachedContent(),
      m_cacheLocker(),

      m_mouseInside(false),

      onClick()
    {
      // Assign the service for this widget.
      setService(std::string("widget"));

      // Assign the input `parent` to this widget: this will also share the engine
      // and events queue if any is defined in the parent widget.
      setParent(parent);
    }

    SdlWidget::~SdlWidget() {
      std::lock_guard<std::recursive_mutex> guard(m_drawingLocker);
      clearTexture();

      std::lock_guard<std::mutex> cacheGuard(m_cacheLocker);
      clearCachedTexture();

      m_names.clear();

      for (WidgetsMap::const_iterator child = m_children.cbegin() ;
           child != m_children.cend() ;
           ++child)
      {
        if (child->widget != nullptr) {
          delete child->widget;
        }
      }
    }

    utils::Uuid
    SdlWidget::draw() {
      // Perform the lock to process oending repaint events.
      handleGraphicOperations();

      // We also need to traverse the list of children and
      // call the `draw` method on each one. This allows to
      // actually perform the pending graphic operations.
      // This will guarantee that repaint operations can
      // bubble up to the top level when needed.
      {
        std::lock_guard<std::recursive_mutex> guard(m_drawingLocker);

        utils::Sizef area = LayoutItem::getRenderingArea().toSize();

        for (WidgetsMap::const_iterator child = m_children.cbegin() ; child != m_children.cend() ; ++child) {
          if (child->widget->isVisible()) {
            child->widget->draw();
          }
        }
      }

      // Return the cached texture.
      std::lock_guard<std::mutex> guard(m_cacheLocker);
      return m_cachedContent;
    }

    void
    SdlWidget::trimEvents(std::vector<engine::EventShPtr>& events) {
      // Traverse the list of events and abalyze each one.
      bool prevWasHide = false;
      bool prevWasShow = false;

      std::vector<engine::EventShPtr>::iterator event = events.begin();

      while (event != events.cend()) {
        // We want to react to specific event which will trigger a
        // modification of the input events queue. These events are
        // the following:
        // 1. Null events will be trashed.
        // 2. Events with None type will be trashed.
        // 3. Hide events will clear the rest of the queue except
        //    Show events: in this case it will be collapsed based
        //    on the current state of the widget.
        // 4. Hide event in an already hidden widget will be trashed.
        // 5. Show event in an already visible widget will be trashed.
        // That'it for now.
        if ((*event) == nullptr || (*event)->getType() == engine::Event::Type::None) {
          event = events.erase(event);
        }
        else if ((*event)->getType() == engine::Event::Type::Hide) {
          // Check whether the item is already hidden: if this is the case we
          // discard this event and move to the next one. We have to keep one
          // of them though, as the `isVisible` status is updated right away
          // even before queuing the event.
          if (prevWasHide) {
            event = events.erase(event);
          }
          else {
            ++event;
          }

          // Mark the fact that we processed a Hide event.
          prevWasHide = true;
        }
        else if ((*event)->getType() == engine::Event::Type::Show) {
          // If this item is already visible, trash it.
          if (prevWasShow) {
            event = events.erase(event);
          }
          else {
            ++event;
          }

          prevWasShow = true;
          prevWasHide = false;
        }
        else {
          // Check whether we were processing a Hide event right before that.
          // If this is the case it means that there's a hide operation with
          // no Show operation: we just need to clear everything apart from
          // the Hide event.
          if (prevWasHide) {
            // Clear the list.
            while (event != events.cend()) {
              event = events.erase(event);
            }
          }
          else {
            // Move to the next event.
            ++event;
          }
        }
      }
    }

    void
    SdlWidget::rebuildZOrdering() {
      // First we need to sort the internal `m_children` array.
      // Note that we want the items to be sorted in ascending
      // order of their z order.
      // Indeed as larger values of z order indicates widgets
      // in front of others, this is the correct behavior to
      // adopt. The sort should compare `lhs` less than `rhs`
      // based on their z order.
      std::sort(m_children.begin(), m_children.end(),
        [](const ChildWrapper& lhs, const ChildWrapper& rhs) {
          return lhs.zOrder < rhs.zOrder;
        }
      );

      // Now rebuild the internal `m_names` array.
      m_names.clear();

      for (int id = 0 ; id < getChildrenCount() ; ++id) {
        m_names[m_children[id].widget->getName()] = id;
      }
    }

    void
    SdlWidget::drawChild(SdlWidget& child,
                         const utils::Sizef& dims)
    {
      const utils::Uuid& uuid = m_content;
      engine::Engine& engine = getEngine();

      // Protect against errors.
      withSafetyNet(
        [&child, &uuid, &engine, &dims]() {
          // Draw this object (caching is handled by the object itself).
          utils::Uuid picture = child.draw();

          // Draw the picture at the corresponding place. Note that the
          // coordinates of the box of each child is in local coordinates
          // relatively to this widget.
          // In order to obtain good results, we need to convert to an
          // intermediate coordinate frame not centered on the origin but
          // rather on the position of this widget.
          // This is because the SDL talks in terms of top left corner
          // and we talk in terms of center.
          // The conversion cannot happen without knowing the dimension
          // of the input texture, which is only known here.
          utils::Boxf render = child.getRenderingArea();

          // Account for the intermediate coordinate frame transformation.
          render.x() += (dims.w() / 2.0f);
          render.y() = (dims.h() / 2.0f) - render.y();

          // TODO: This idea actually prevents to have children spanning
          // areas beyong the parent area.
          engine.drawTexture(
            picture,
            nullptr,
            &uuid,
            &render
          );
        },
        std::string("draw_child(") + child.getName() + ")"
      );
    }

    bool
    SdlWidget::refreshEvent(const engine::Event& e) {
      // The refresh event is meant to allow the update of the internal
      // cached content with the up-to-date actual content. However this
      // includes creating a new texture or destroying any existing
      // cached content and due to limitations of the engine is meant to
      // be done in the main thread.
      // As we cannot guarantee that this operation will be performed in
      // the main thread we chose to save internally such events to process
      // them later on, during a call to `draw` method which is surely
      // called from the main thread.
      // So in here we just have to save the event for further processing.

      // If no previous refresh operations were registered, we need to
      // create a new one.
      if (m_refreshOperation == nullptr) {
        m_refreshOperation = std::make_shared<engine::Event>(e);
      }
      else {
        // Might happen if events are posted faster than the repaint from
        // the main thread occurs. Should not happen too often if the fps
        // for both the repaint and events system are set to work well
        // together.
        m_refreshOperation->merge(e);
      }

      // Use base handler to determine whether the event was recognized.
      return LayoutItem::refreshEvent(e);

    }

    bool
    SdlWidget::repaintEvent(const engine::PaintEvent& e) {
      // Usually the paint event is meant to update the internal
      // visual representation of this widget. It is important so
      // that the display for this widget is always up-to-date
      // with its content.
      // Unfortunately some limitations in the engine we're using
      // make it impossible to create textures outside of the
      // main thread. This is particularly problematic because
      // the whole events' system is designed to handle easily
      // the repaint operation just like any operation.
      // We chose to use a workaround: we save the repaint events
      // in an internal array so that they can be processed later
      // on during a call to the `draw` method. This method is
      // called by the application into which the widget is used
      // and always from the main thread.
      // We handle caching and the repaint operation itself over
      // there.
      // So in here we just have to save the event for further
      // processing.

      // If no previous repaint operations were registered, we need to
      // create a new one.
      if (m_repaintOperation == nullptr) {
        m_repaintOperation = std::make_shared<engine::PaintEvent>(e);
      }
      else {
        // Might happen if events are posted faster than the repaint from
        // the main thread occurs. Should not happen too often if the fps
        // for both the repaint and events system are set to work well
        // together.
        m_repaintOperation->merge(e);
      }

      // Trigger a refresh.
      requestRefresh();

      // Use base handler to determine whether the event was recognized.
      return LayoutItem::repaintEvent(e);
    }

    bool
    SdlWidget::resizeEvent(engine::ResizeEvent& e) {
      // Use the base handler to handle the resize.
      const bool toReturn = LayoutItem::resizeEvent(e);

      // We should clear the existing repaint events, as
      // the sizes associated to them have probably become
      // obsolete due to the resize event.
      // And in any case we also issue a new repaint event
      // so nothing should be lost.

      // First clear internal repaint/refresh operations.
      m_repaintOperation.reset();
      m_refreshOperation.reset();

      // Clear existing events as well.
      removeEvents(engine::Event::Type::Repaint);
      removeEvents(engine::Event::Type::Refresh);

      // Mark the content as dirty.
      makeContentDirty();

      // Return the value provided by the base handler.
      return toReturn;
    }

    bool
    SdlWidget::zOrderChanged(const engine::Event& e) {
      // Traverse the children list and updtae the z order for each one.
      for (WidgetsMap::iterator child = m_children.begin() ; child != m_children.end() ; ++child) {
        child->zOrder = child->widget->getZOrder();
      }

      // Proceed to rebuild the z ordering.
      rebuildZOrdering();

      // Use the base handler method to provide a return value.
      return LayoutItem::zOrderChanged(e);
    }

    void
    SdlWidget::refreshEventPrivate(const engine::Event& /*e*/) {
      // Replace the cached content.
      std::lock_guard<std::mutex> guard(m_cacheLocker);

      // Create a new cached texture if the size of the cached content is
      // different from the current size of the content.
      utils::Sizef old;
      if (m_cachedContent.valid()) {
        old = getEngine().queryTexture(m_cachedContent);
      }
      utils::Sizef cur = getEngine().queryTexture(m_content);

      if (!m_cachedContent.valid() || old != cur) {
        // Clear existing cached texture.
        clearCachedTexture();

        // Create new one with required dimensions.
        m_cachedContent = createContentPrivate();

        // In order to make the texture valid for rendering we need to clear it
        // with a valid color.
        getEngine().fillTexture(m_cachedContent, getPalette());
      }
      else {
        // Clear content so that we do not get polluted by the remains of old
        // renderings.
        clearContentPrivate(m_cachedContent, utils::Boxf::fromSize(old, true));
      }

      // Copy the data of `m_content` onto `m_cachedContent`.
      // We can copy withtout specifying dimensions as both
      // textures should have similar sizes.
      getEngine().drawTexture(m_content, nullptr, &m_cachedContent);
    }

    void
    SdlWidget::repaintEventPrivate(const engine::PaintEvent& e) {
      // When calling this method we should be in the main thread.
      // This means that it is ok to create a texture, it will be
      // usable in the main thread for display purposes.
      // So in order to repaint the widget, a valid rendering area
      // must have been defined through another process (usually
      // by updating the layout of the parent widget). If this is
      // not the case, an error is raised. Also the widget should
      // be visible: if this is not the case we know that the
      // `setVisible` method will trigger a repaint when called
      // with a `true` value (i.e. when the widget is set back to
      // visible). So no need to worry of these events if the
      // widget is not visible.
      // We also need to handle caching of the data so that it can
      // be reused later on withtout modifications and need to
      // redraw everything.

      // So first check that the widget is visible.
      if (!isVisible()) {
        // Return early.
        return;
      }

      // Retrieve and check the rendering area for this widget.
      utils::Boxf area = LayoutItem::getRenderingArea();

      if (!area.valid()) {
        error(std::string("Could not repaint widget"), std::string("Invalid size"));
      }

      // We are certain that the repaint operation is valid. In order
      // to perform the repaint we need to either completely recreate
      // the content or only update part of it.
      // The information about the area to repaint is available in the
      // input event but it's not enough. Indeed we are able to determine
      // whether only a part of the widget should be updated but not
      // whether the widget needs to be recreated. This information is
      // describes internally with the `m_contentDirty` boolean.
      // Checking it will allow us to precisely determine whether a global
      // paint event should also be paired with the creation of a new
      // texture for this widget or the content can only be redrawn.
      const bool redraw = m_contentDirty;

      if (m_contentDirty) {
        // Clear the internal texture.
        clearTexture();

        // Create the new content.
        m_content = createContentPrivate();

        // Until further notice the content is up-to-date.
        m_contentDirty = false;
      }

      // Perform the update of the area described by the input
      // paint event.
      const std::vector<utils::Boxf> regions = e.getUpdateRegions();

      for (int id = 0 ; id < static_cast<int>(regions.size()) ; ++id) {
        const utils::Boxf region = mapFromGlobal(regions[id]);

        log("Updating region " + region.toString() + " from " + regions[id].toString() + " (ref: " + area.toString() + ")");

        clearContentPrivate(m_content, region);
        drawContentPrivate(m_content, region);
      }

      // Copy also the internal area in order to perform the coordinate
      // frame transform.
      utils::Sizef dims = area.toSize();

      // Proceed to update of children containers if any: at this point
      // the `m_children` array is already sorted by z order so we can
      // just iterate over it and we will process children in a valid
      // order.
      // In addition to that we only need to repaint children which have
      // a non empty intersection with any of the regions to update.
      // This behavior might be overriden by the `redraw` operation as
      // obviously if the whole widget has been recreated we need to
      // repaint chidlren.
      for (WidgetsMap::const_iterator child = m_children.cbegin() ; child != m_children.cend() ; ++child) {
        // The chidlren needs to be repainted if:
        // 1. It is visible.
        // 2. It needs a repaint from the input `event`.
        // 3. It needs a repaint because the widget has been recreated.
        // The only tricky part is determining whether the widget has
        // some intersection with any of the update regions.

        bool intersectWithRepaint = false;
        int id = 0;
        const utils::Boxf childBox = child->widget->getRenderingArea();

        while (id < static_cast<int>(regions.size()) && !intersectWithRepaint) {
          // Convert region from global to local.
          const utils::Boxf region = mapFromGlobal(regions[id]);

          // Determine whether the region has an intersection with the child.
          intersectWithRepaint = region.intersect(childBox).valid();

          // Move to the next one.
          ++id;
        }

        // Check whether we should repaint this child.
        if (child->widget->isVisible() && (intersectWithRepaint || redraw)) {
          drawChild(*child->widget, dims);
        }
      }

      ////////////////////////////
      // TODO: Does this work ? //
      ////////////////////////////

      // Now we updated children and recreated our internal content if needed.
      // We can now notify the parent widget (if any) to update itself with
      // our up-to-date content.
      // One last thing to worry about is if any of the areas specified in the
      // paint event overflows the area defined for `this` widget.
      // If this is the case it means that we potentially need to udpate other
      // siblings elements of this widget with some content from this widget.
      // At this point the fact that we have a parent or not is important: indeed
      // if we have a parent we can just proceed to post a repait event for the
      // parent and we should be good. Indeed either the parent covers all the
      // areas defined by the input paint event in which case it can handle all
      // of them or it is not the case and whatever decision we could make can
      // also be done by the parent.
      // The only problem is when we don't have any parent. In this case we can
      // either stop the process, which means that we potentially skipped parts
      // of the repaint process or try to notify siblings of `this` widget in
      // order to trigger a repaint on their behalf.
      // This is made possible if `this` widget has a layout: in this case it
      // is the best candidate to transmit the paint event to siblings of `this`
      // widget.
      //
      // Finally we also need to take into consideration the fact that `this`
      // widget might have been completely repainted. If this is the case, we
      // need to add the whole widget's area to the repaint event. The event
      // will handle internally the necessary merge operation to remove the
      // regions which might be redundant after that.

      // Determine whether a parent is available for this widget: if this
      // is the case we need to transmit the repaint event to it as we just
      // updated this widget. This most likely has an impact on the parent
      // widget so we should notify it so that it can performs its own
      // update process.
      if (hasParent()) {
        // Note that we need to create a new event as the one we're using is
        // not a pointer but a refernece so we cannot directly post it to any
        // other object.
        engine::PaintEventShPtr ne = std::make_shared<engine::PaintEvent>(e);

        // Assign the emitter of this event.
        ne->setEmitter(this);

        // Add the region covered by this widget to the repaint event if it has
        // been redrawn.
        if (redraw) {
          log("Adding redraw region " + mapToGlobal(area, false).toString() + " to repaint event containing " + std::to_string(ne->getUpdateRegions().size()) + " region(s)");
          ne->addUpdateRegion(mapToGlobal(area, false));
        }

        // Assign receiver and post the event.
        ne->setReceiver(m_parent);
        m_parent->postEvent(ne);

        // All is well.
        return;
      }

      // No parent is available for this widget. This usually means that we
      // reached the top of the widgets' hierarchy and that no matter the
      // event which started this process in the first place we now have once
      // again a nice up-to-date cache of all widgets.
      // So most of the time this would indicate that we're done.
      // There's one special case though: if one or more of the update regions
      // overflows this widget (i.e. covers area which are beyond the bounds
      // of this widget) we might want to notify siblings elements of `this`
      // widget so that they can repaint tehmselves.
      // This process is only available through posting a repaint event on the
      // layout of `this` widget if any: indeed the layout is the best option
      // to reach siblings as it is likely that is does not manage only `this`
      // widget but also its siblings.
      //
      // This process is only relevant if a layout is defined for this widget,
      // otherwise we have no idea about the whereabouts of `this` widget's
      // siblings which is kinda sad.
      if (!isManaged()) {
        // Nothing more we can do, no information about the siblings of this
        // widget so even in the eventuality where we find some areas which
        // are not fully covered by this widget we can't do anything. Maybe
        // broadcasting the repaint event would be nice but not sure we want
        // to do that for now.
        return;
      }

      // So first, keep only the regions which are not completely included in
      // `this` widget's area.
      engine::PaintEventShPtr ne = std::make_shared<engine::PaintEvent>(this);

      const utils::Boxf global = mapToGlobal(area, false);

      for (int id = 0 ; id < static_cast<int>(regions.size()) ; ++id) {
        // Check whether the intersection of this region with the area defined
        // for this widget is equal to the input area: if this is the case it
        // means that the area is contained inside `this` widget area, otherwise
        // it means that at least part of it lies outside of the reference area.
        const utils::Boxf ref = regions[id].intersect(global);

        if (ref != regions[id]) {
          log("Area " + regions[id].toString() + " does not fit into widget's area of " + area.toString() + ", propagating to manager layout");
          // The area has part of it which do not lie in `this` widget's area.
          ne->addUpdateRegion(ref);
        }
      }

      // Post the event to the director's layout queue.
      ne->setReceiver(getManager());
      getManager()->postEvent(ne);
    }

  }
}
