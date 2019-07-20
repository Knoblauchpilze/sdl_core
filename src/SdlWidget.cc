
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
      m_repaints(),
      m_childrenLocker(),

      m_layout(),
      m_palette(engine::Palette::fromButtonColor(color)),
      m_engine(nullptr),

      m_parent(nullptr),

      m_contentDirty(true),
      m_mouseInside(false),
      m_zOrder(0),

      m_content(),
      m_repaintOperation(nullptr),
      m_contentLocker(),

      m_cachedContent(),
      m_cacheLocker(),

      onClick()
    {
      // Assign the service for this widget.
      setService(std::string("widget"));

      // Assign the input `parent` to this widget: this will also share the engine
      // and events queue if any is defined in the parent widget.
      setParent(parent);
    }

    SdlWidget::~SdlWidget() {
      {
        Guard guard(m_contentLocker);
        clearTexture();

        Guard cacheGuard(m_cacheLocker);
        clearCachedTexture();
      }

      {
        Guard guard(m_childrenLocker);

        m_names.clear();

        for (WidgetsMap::const_iterator child = m_children.cbegin() ;
            child != m_children.cend() ;
            ++child)
        {
          if (child->widget != nullptr) {
            delete child->widget;
          }
        }

        m_repaints.clear();
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
        utils::Sizef area = getRenderingArea().toSize();

        Guard guard(m_childrenLocker);

        for (WidgetsMap::const_iterator child = m_children.cbegin() ; child != m_children.cend() ; ++child) {
          if (child->widget->isVisible()) {
            child->widget->draw();
          }
        }
      }

      // Return the cached texture.
      Guard guard(m_cacheLocker);
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
    SdlWidget::drawWidget(SdlWidget& widget,
                          const utils::Boxf& src,
                          const utils::Boxf& dst)
    {
      const utils::Uuid& uuid = m_content;
      engine::Engine& engine = getEngine();

      log("Drawing area " + src.toString() + " from widget " + widget.getName() + " to " + dst.toString());

      // Protect against errors.
      withSafetyNet(
        [&widget, &uuid, &engine, &src, &dst]() {
          // Retrieve a texture identifier representing the `widget` to draw.
          utils::Uuid picture = widget.draw();

          // Draw the texture at the specified coordinates.
          engine.drawTexture(
            picture,
            &src,
            &uuid,
            &dst
          );
        },
        std::string("draw_child(") + widget.getName() + ")"
      );

      // Register this widget with the time stamp of the repaint operation.
      // This will help ignoring repaint events which we might receive from
      // this widget.
      // We first need to determine whether the element to repaint belongs
      // to the children of this widget.
      if (hasChild(widget.getName())) {
        m_repaints[widget.getName()] = std::chrono::steady_clock::now();
      }
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
      // there. So in here we just have to save the event for
      // further processing.
      // One important thing to notice though is the origin of
      // the paint event. Paint event might have two main origin:
      // either directly from this widget (typically in the case
      // of a resize event) or from children (typically because
      // they have updated themselves and notify the parent to
      // do the same).
      // Children events should be handled with care because they
      // might be generated by processing occurring in the parent.
      // We keep an internal table `m_repaints` which indicates
      // the last paint operation of any child on the content of
      // this widget.
      // Basically any event generated by the children which is
      // anterior to the last paint operation can be discarded
      // as the content of this widget is already up-to-date.

      // First check whether this paint event has been emitted
      // by a child of this widget.
      if (e.getEmitter()) {
        // Search for the emitter of the event into the internal
        // chidlren array.
        const std::string& name = e.getEmitter()->getName();
        SdlWidget* widget = getChildOrNull<SdlWidget>(name);

        // Check whether we could find it.
        if (widget != nullptr) {
          // Try to determine whether the timestamp of the paint
          // event is anterior to the last repaint operation for
          // this widget.
          RepaintMap::const_iterator lastRepaint = m_repaints.find(name);

          if (lastRepaint != m_repaints.cend()) {
            // Check timestamps and see if the events is posterior
            // to the last repaint operation. If this is not the
            // case we can trash the event.
            if (lastRepaint->second >= e.getTimestamp()) {
              log("Trashing repaint from " + name + " posterior to last refresh", utils::Level::Info);
              // Use base handler to provide a return value.
              return LayoutItem::repaintEvent(e);
            }
          }
        }
      }

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

      // Clear existing events as well.
      removeEvents(engine::Event::Type::Repaint);

      // Mark the content as dirty.
      makeContentDirty();

      // Return the value provided by the base handler.
      return toReturn;
    }

    bool
    SdlWidget::zOrderChanged(const engine::Event& e) {
      Guard guard(m_childrenLocker);

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
    SdlWidget::refreshPrivate(const engine::PaintEvent& e) {
      // Replace the cached content.
      Guard guard(m_cacheLocker);

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

      // So the cached content is now up-to-date with the real content of this
      // widget. We can now notify the parent widget or layout about the fact
      // that we've been updateing ourselves so that they can perform the needed
      // repaint operations if needed.
      // As the size of `this` widget might have changed between the two repaint
      // operations, we need to notify the parent with the largest available
      // area between the old size and the new one.
      // This will allow the parent to update both the area currently occupied
      // by the widget but also the old area previously occupied by the widget
      // and not anymore (in case `this` widget has shrunk).
      // Not only that but we also need to get the largest area among all the
      // available ones which are both the old and current size of the widget but
      // also the available paint areas registered in the input paint event.
    
      // Create the maximum area between the old and current size.
      const float w = old.w() > cur.w() ? old.w() : cur.w();
      const float h = old.h() > cur.h() ? old.h() : cur.h();

      const utils::Boxf local(0.0f + (w - cur.w()) / 2.0f, 0.0f - (h - cur.h()) / 2.0f, w, h);
      const utils::Boxf toRepaint = mapToGlobal(local);

      // Once we have the coordinates, create the paint event.
      engine::PaintEventShPtr pe = std::make_shared<engine::PaintEvent>(toRepaint, nullptr);
      pe->setEmitter(this);

      // Don't forget to add the input paint regions. We need to do that only if
      // the event does not come from the element we want to send it to. As an
      // example we don't really need to notify the parent widget that a region
      // has been updated if it is the one which told us in the first place.
      // The copy is handled on the fly when building the output event.
      if (e.getEmitter() != nullptr && hasChild(e.getEmitter()->getName())) {
        pe->copyUpdateRegions(e);
      }

      // Determine the object to which is should be sent: either the parent widget
      // or the manager layout. We only choose the manager layout if the paint event
      // contains update areas larger than this widget. Indeed otherwise there's no
      // need to notify siblings that this widget has been updated as all changes are
      // contained inside it.
      const utils::Boxf global = mapToGlobal(LayoutItem::getRenderingArea(), false);
      EngineObject* o = nullptr;

      // Check for a parent widget or if no such object exist a manager layout.
      if (hasParent()) {
        pe->setReceiver(m_parent);
        o = m_parent;
      }
      else if (isManaged() && !pe->isContained(global)) {
        pe->setReceiver(getManager());
        o = getManager();
      }

      if (o == nullptr) {
        log("Do not post repaint event, no need to do so", utils::Level::Info);
      }

      // Post the event if we have an object where to post it.
      if (o != nullptr) {
        o->postEvent(pe, false, false);
      }
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

        log("Updating region " + region.toString() + " from " + regions[id].toString() + " (ref: " + area.toString() + ") (source: " + e.getEmitter()->getName() + ")");

        // TODO: Handle the case where the source of the event does not
        // come from one of our child this probably means that we need
        // to repaint the source onto this widget.

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
      Guard guard(m_childrenLocker);

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
          intersectWithRepaint = region.intersects(childBox);

          // Move to the next one.
          ++id;
        }

        // Check whether we should repaint this child.
        if (child->widget->isVisible() && (intersectWithRepaint || redraw)) {
          // Compute the `src` and `dst` areas to draw the widget.
          // We want to draw the entirety of the widget's texture
          // at its expected position in this widget.
          const utils::Boxf dst = convertToEngineFormat(childBox, utils::Boxf::fromSize(dims));

          drawWidget(*child->widget, utils::Boxf::fromSize(childBox.toSize()), dst);
        }
      }

      // Finally let's handle the repaint of the source of the repaint event
      // if it is not part of our children. This allows to actually display
      // elements on top of other widgets.
      if (e.getEmitter() != nullptr && !hasChild(e.getEmitter()->getName()) && e.getEmitter() != this) {
        // Check whether the emitter can be displayed as a widget.
        SdlWidget* source = dynamic_cast<SdlWidget*>(e.getEmitter());

        if (source != nullptr) {
          // Draw all the repaint regions mentionned in the event using the
          // `source` of the event as repaint base.
          // For each area described in the paint event we need to compute
          // its intersection with `this` object: from that we can derive
          // the `src` area to repaint. The `dst` area corresponds to the
          // local conversion of the `regions[id]` box. Strictly speaking we
          // could handle the intersection of this area with the dimensions
          // of `this` widget's area in order to only blit relevant parts
          // of the `source` object.
          const utils::Boxf global = source->getDrawingArea();

          for (int id = 0 ; id < static_cast<int>(regions.size()) ; ++id) {
            // Convert the input region expressed in global coordinate frame
            // into local frame.
            const utils::Boxf region = mapFromGlobal(regions[id]);

            // The `dst` region of the repaint area corresponds to this region
            // converted into engine format. Indeed the `region` is already in
            // local coordinate frame.
            const utils::Boxf dst = convertToEngineFormat(region, utils::Boxf::fromSize(dims));

            // Compute the intersection of the regions with `this` object's
            // area and convert it to global coordinate frame.
            const utils::Boxf inter = utils::Boxf::fromSize(dims, true).intersect(region);
            const utils::Boxf interG = mapToGlobal(inter);
            const utils::Boxf src = convertToLocal(interG, global);

            log("Drawing " + source->getName() + " to " + dst.toString() + " from " + src.toString());
            drawWidget(*source, src, region);
          }
        }
      }

      // Now perform the refresh operation.
      refreshPrivate(e);
    }

  }
}
