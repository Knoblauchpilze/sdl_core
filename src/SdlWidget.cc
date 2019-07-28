
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
      m_childrenRepaints(),
      m_repaint(),
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

        m_childrenRepaints.clear();
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
      return getContentUuid();
    }

    bool
    SdlWidget::drawOn(const utils::Uuid& on,
                      const utils::Boxf* src,
                      const utils::Boxf* dst)
    {
      // The point of this `drawOn` method is to draw the relevant content
      // of this widget on the specified target. If this widget does not
      // cover the desired `src` area we need to transmit the request to
      // children in order to find the one covering the area.
      Guard guard(m_cacheLocker);

      // If this widget is hidden, do nothing.
      if (!isVisible()) {
        return false;
      }

      // First, handle the case where `src` is null: in this case we just
      // want to draw the whole internal texture on the `on` texture at
      // the specified `dst` position.
      if (src == nullptr) {
        getEngine().drawTexture(m_cachedContent, src, &on, dst);

        // We're done.
        return true;
      }

      // Now we now that `src` contains a value, we need to check whether
      // it is covered by this widget. If this is the case we can just
      // draw it at the desired position on the `on` texture and we're done.
      const utils::Boxf spanned = getRenderingArea().toOrigin();
      const utils::Boxf inter = spanned.intersect(*src);

      if (inter.valid()) {
        // Draw the internal content at the specified position and call
        // it done. We need to only draw the area which intersects the
        // actual `src` area.
        log("Widget contains area " + src->toString() + " (total: " + spanned.toString() + ", intersect: " + inter.toString() + ")");

        const utils::Boxf srcEngine = convertToEngineFormat(inter, spanned);

        getEngine().drawTexture(m_cachedContent, &srcEngine, &on, dst);
        return true;
      }

      // The `src` area is not spanned by this widget, we need to transmit
      // the request to the children so that we can see if one of them can
      // handle the request.
      bool drawn = false;
      {
        Guard cguard(m_childrenLocker);
        for (WidgetsMap::const_iterator child = m_children.cbegin() ; child != m_children.cend() ; ++child) {
          // Do not request children which are not visible.
          if (!child->widget->isVisible()) {
            continue;
          }

          // Convert the `src` area in terms of child coordinate frame and
          // perform the draw on operation.
          const utils::Boxf childSrc = convertToLocal(*src, child->widget->getRenderingArea());
          log("Requesting child " + child->widget->getName() + " with area " + childSrc.toString() + " (from " + src->toString() + ", child: " + child->widget->getRenderingArea().toString() + ")");
          const bool valid = child->widget->drawOn(on, &childSrc, dst);

          // Update the status boolean.
          if (valid) {
            drawn = true;
          }
        }
      }

      // Return the status indicating whether some elements could be drawn.
      return drawn;
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

    bool
    SdlWidget::focusInEvent(const engine::FocusEvent& e) {
      log("Handling focus in from " + e.getEmitter()->getName() + " with reason " + std::to_string(static_cast<int>(e.getReason())) + " (policy: " + getFocusPolicy().toString() + ")");

      // A focus in event has been raised with a specific reason. The first step
      // in processing this event is to actually determine whether this widget is
      // sensitive to this kind of focus events: if this is not the case we do
      // not want to redraw the content or anything, just perform the notification
      // of this event to the rest of the component to update the needed states.
      // If the focus event is supported we need to trigger a redraw of the
      // widget's content if needed in order for it to match its current focus
      // status.
      // We should also trigger the generation of a `GainFocus` event which will
      // handle the propagation of this focus event to parent and children so
      // that they get notified of the new status for this widget.

      // If the focus event has a hover event as a cause, update the position of
      // the mouse. This should be done no matter if this widget will actually
      // handle the focus reason.
      if (e.getReason() == engine::FocusEvent::Reason::HoverFocus) {
        // The mouse is now inside this widget.
        m_mouseInside = true;
      }

      // Determine whether the focus reason provided by the input event is
      // supported by this widget.
      if (canHandleFocusReason(e.getReason())) {
        // Now that we know the focus reason can be handled, we need to update the
        // widget's content to match the new focus state. Once again use the dedicated
        // handler.
        updateStateFromFocus(e.getReason(), true);
      }

      // Post a gain focus first to this widget (so that potential children
      // which are currently focused get deactivated) which will then be
      // transmitted to the parent so that it can do the necessary updates
      // regarding siblings of `this` widget which may be focused.
      postEvent(std::make_shared<engine::FocusEvent>(e.getReason()));

      // Use the base handler to provide a return value.
      return LayoutItem::focusInEvent(e);
    }

    bool
    SdlWidget::focusOutEvent(const engine::FocusEvent& e) {
      log("Handling focus out from " + e.getEmitter()->getName() + " with reason " + std::to_string(static_cast<int>(e.getReason())));

      // A focus out event has been raised with a specific reason. The process to
      // follow is very similar to the one used in `focusInEvent` except the event
      // we will generate will be a `LostFocus` instead of a `GainFocus`.
      // See this function for more details.

      // If the focus event has a hover event as a cause, update the position of
      // the mouse. This should be done no matter if this widget will actually
      // handle the focus reason.
      if (e.getReason() == engine::FocusEvent::Reason::HoverFocus) {
        // The mouse is now outside this widget.
        m_mouseInside = false;
      }

      // Determine whether we can handle this reason.
      if (!canHandleFocusReason(e.getReason())) {
        // Use the base handler to provide a return value.
        return LayoutItem::focusInEvent(e);
      }

      // Update the internal state.
      updateStateFromFocus(e.getReason(), false);

      // Post the `LostFocus` event.
      postEvent(std::make_shared<engine::Event>(engine::Event::Type::LostFocus));

      // Use the base handler to provide a return value.
      return LayoutItem::focusOutEvent(e);
    }

    bool
    SdlWidget::gainFocusEvent(const engine::FocusEvent& e) {
      // This type of event is triggered by children widget in case they
      // just gained focus. The event's source should thus be a child
      // widget.
      // This method needs to unfocus any other child widget for `this`
      // widget which may have the focus and also notify the parent widget
      // or the manager layout if any with the fact that this widget has
      // now focus.
      // We also need to focus ourselves so that the chain of widgets
      // which lead to the deepest focused child can be built.
      log("Handling gain focus from " + e.getEmitter()->getName() + " with reason " + std::to_string(static_cast<int>(e.getReason())));

      // Apply the focus modification if needed: we also optimize a bit
      // by checking whether the event is produced by `this` widget: if
      // this is the case we don't need to update anything as it has most
      // likely already been handled.
      if (e.getEmitter() != this && canHandleFocusReason(e.getReason())) {
        // Now that we know the focus reason can be handled, we need to update the
        // widget's content to match the new focus state. Once again use the dedicated
        // handler.
        updateStateFromFocus(e.getReason(), true);
      }

      // Traverse the internal array of children and unfocus any widget
      // which is not the source of the event.
      {
        Guard guard(m_childrenLocker);
        for (WidgetsMap::const_iterator child = m_children.cbegin() ; child != m_children.cend() ; ++child) {

          log("Child " + child->widget->getName() + (child->widget->hasFocus() ? " has " : " has not ") + "focus");
          // If the child is not the source of the event and is focused, unfocus it.
          if (e.getEmitter() != child->widget && child->widget->hasFocus()) {
            log("Posting focus out event on " + child->widget->getName() + " due to " + e.getEmitter()->getName() + " gaining focus");
            postEvent(std::make_shared<engine::FocusEvent>(false, e.getReason(), child->widget), false, true);
          }
        }
      }

      // Transmit the gain focus event to the parent widget if any or the
      // manager layout.
      engine::FocusEventShPtr gfe = std::make_shared<engine::FocusEvent>(e.getReason());
      EngineObject* o = nullptr;

      if (hasParent()) {
        gfe->setReceiver(m_parent);
        o = m_parent;
      }
      else if (isManaged()) {
        gfe->setReceiver(getManager());
        o = getManager();
      }

      if (o == nullptr) {
        log("Do not post gain focus event, no need to do so", utils::Level::Info);
      }

      if (o != nullptr) {
        postEvent(gfe, false, true);
      }

      // Use the base handler to provide a return value.
      return LayoutItem::gainFocusEvent(e);
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
      // the paint event. Paint event might have two main origins:
      // either directly from this widget (typically in the case
      // of a resize event) or from children (typically because
      // they have updated themselves and notify the parent to
      // do the same).
      // In both cases these events should be handled with care as
      // the repaint operation is significant and we want to be
      // absolutely certain that such an operation is needed.
      // To determine whether it's needed or not we will use the
      // internal repaint timestamp which allows when compared
      // with the event's timestamp to determine whether the last
      // repaint operation is posterior to the event's creation
      // date. In addition to that we will also compare the internal
      // timestamp at which the widget has completely been redrawn
      // with its internal repaint timestamp.
      // Note that we assume here that when an event comes from a
      // widget it at least contains all its area.

      // Compare both timestamps and see whether we need to
      // consider this event or if we can safely trash it.
      if (m_repaint >= e.getTimestamp()) {
        // The event has been produced before the last repaint
        // operation of this widget. This is a good sign that the
        // event could be ignored.
        // In order to be sure we need to check whether the repaint
        // timestamp of the emitter is posterior to the internal
        // repaint timestamp held in this widget.
        // If this is the case it means that the widget has been
        // repainted after the last time we drew it completely so
        // we can use this event to update things.
        if (e.getEmitter() != nullptr) {
          const std::string name = e.getEmitter()->getName();

          // Retrieve the internal timestamp if any.
          RepaintMap::const_iterator lastRepaint = m_childrenRepaints.find(name);

          if (lastRepaint != m_childrenRepaints.cend() && lastRepaint->second >= e.getTimestamp()) {
            // We repainted this widget after the event has been emitted,
            // no need to paint it again.
            log("Trashing repaint from " + e.getEmitter()->getName() + " posterior to last refresh", utils::Level::Info);

            // Use base handler to provide a return value.
            return LayoutItem::repaintEvent(e);
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
        EngineObject* em = m_repaintOperation->getEmitter();
        m_repaintOperation->merge(e);

        // Also, assign this event's emitter to `this` if both sources are
        // not equal.
        if (em != e.getEmitter()) {
          m_repaintOperation->setEmitter(this);
        }
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

      // Update the last repaint which just took place right now.
      m_repaint = std::chrono::steady_clock::now();

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
      if (e.getEmitter() != nullptr && (e.getEmitter() == this || hasChild(e.getEmitter()->getName()))) {
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
        postEvent(pe, false, false);
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

      // Perform the update of the area described by the input paint event.
      // To do so we need to update the content of `this` widget in the input
      // update areas but also redraw the children which intersect these
      // locations.
      // Not that in order to redraw only what's really necessary we process
      // for each region in a single pass both the update of `this` widget's
      // content and the update of the children intersecting this area.
      const std::vector<utils::Boxf> regions = e.getUpdateRegions();

      utils::Sizef dims = area.toSize();

      for (int id = 0 ; id < static_cast<int>(regions.size()) ; ++id) {
        // Convert the region from global to local coordinate frame.
        const utils::Boxf region = mapFromGlobal(regions[id]);

        // log("Updating region " + region.toString() + " from " + regions[id].toString() + " (ref: " + area.toString() + ") (source: " + e.getEmitter()->getName() + ")");

        // Update the content of `this` widget: first clear the content and
        // then perform the draw operation.
        clearContentPrivate(m_content, region);
        drawContentPrivate(m_content, region);

        // Now iterate over children and draw them if needed (i.e. if they
        // intersect the current area).
        {
          Guard guard(m_childrenLocker);

          for (WidgetsMap::const_iterator child = m_children.cbegin() ; child != m_children.cend() ; ++child) {
            // The child needs to be repainted if:
            // 1. It is visible.
            // 2. It needs a repaint from the input `event`.
            // 3. It needs a repaint because the widget has been recreated.

            // If the widget is not visible, skip this part entirely.
            if (!child->widget->isVisible()) {
              continue;
            }

            // Determine whether this widget intersect the current update region.
            const utils::Boxf childBox = child->widget->getRenderingArea();
            utils::Boxf dst = region.intersect(childBox);
            utils::Boxf dstEngine = convertToEngineFormat(dst, area);

            // If this widget does not intersect the current region, do nothing.
            // This behavior is overriden by the `redraw` boolean which indicates
            // that as `this` widget has been recreated we will repaint children
            // no matter what.
            if (!dst.valid() && !redraw) {
              continue;
            }

            // Determine the source area by converting the `dst` area into
            // the widget's coordinate frame.
            const utils::Boxf src = convertToLocal(dst, childBox);
            const utils::Boxf srcEngine = convertToEngineFormat(src, childBox);

            // log("Drawing child " + child->widget->getName() + " (src: " + src.toString() + ", dst: " + dst.toString() + "), intersect with " + region.toString());
            drawWidget(*child->widget, srcEngine, dstEngine);

            // Update the repaint timestamp for this child if the area contains
            // the child's area.
            if (region.contains(childBox)) {
              m_childrenRepaints[child->widget->getName()] = std::chrono::steady_clock::now();
            }
          }
        }
      }

      Guard guard(m_childrenLocker);

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
            const utils::Boxf interD = utils::Boxf::fromSize(dims, true).intersect(region);
            const utils::Boxf dst = convertToEngineFormat(interD, utils::Boxf::fromSize(dims));

            // Compute the intersection of the regions with `this` object's
            // area and convert it to global coordinate frame.
            const utils::Boxf inter = utils::Boxf::fromSize(dims, true).intersect(region);
            const utils::Boxf interG = mapToGlobal(inter);
            const utils::Boxf src = convertToLocal(interG, global);

            log("Drawing " + source->getName() + " from " + src.toString() + " to " + dst.toString() + " (raw: " + interD.toString() + ")", utils::Level::Info);
            drawWidgetOn(*source, m_content, src, dst);
          }
        }
      }

      // Now perform the refresh operation.
      refreshPrivate(e);
    }

    void
    SdlWidget::drawWidget(SdlWidget& widget,
                          const utils::Boxf& src,
                          const utils::Boxf& dst)
    {
      const utils::Uuid& uuid = m_content;
      engine::Engine& engine = getEngine();

      // Protect against errors.
      withSafetyNet(
        [&widget, &uuid, &engine, &src, &dst]() {
          // Retrieve a texture identifier representing the `widget` to draw.
          utils::Uuid picture = widget.draw();

          // Draw the texture at the specified coordinates.
          engine.drawTexture(picture, &src, &uuid, &dst);
        },
        std::string("drawWidget(") + widget.getName() + ")"
      );
    }

    void
    SdlWidget::drawWidgetOn(SdlWidget& widget,
                            const utils::Uuid& on,
                            const utils::Boxf& src,
                            const utils::Boxf& dst)
    {
      bool span = false;

      // Protect against errors.
      withSafetyNet(
        [&widget, &on, &src, &dst, &span]() {
          // Display the widget on the input texture at the specified coordinates.
          span = widget.drawOn(on, &src, &dst);
        },
        std::string("drawWidgetOn(") + widget.getName() + ")"
      );

      if (!span) {
        log("Widget " + widget.getName() + " does not seem to span area " + src.toString(), utils::Level::Warning);
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

    bool
    SdlWidget::filterMouseEvents(const EngineObject* watched,
                                 const engine::EventShPtr e) const noexcept
    {
      // Check whether the input event is an instance of a mouse event: if
      // this is not the case we won't filter it.
      engine::MouseEventShPtr me = std::dynamic_pointer_cast<engine::MouseEvent>(e);
      if (me == nullptr) {
        return false;
      }

      // Based on the type of mouse event the filtering conditions vary a
      // little.
      // In the case a mouse move we just want to activate the widget that
      // is more specific. This means that we will try to determine whether
      // the `watched` object should receive the event or if another more
      // relevant children widget can receive it.
      // A more relevant widget is found if it spans the area of the mouse
      // move and has a higher z order.
      //
      // In the case of a mouse click the the process is similar except we
      // are considering the position of the click instead of the mouse
      // motion.

      // Retrieve the mouse position and convert it to local coordinate
      // frame.
      const utils::Vector2f lpos = mapFromGlobal(me->getMousePosition());

      // We can now traverse the list of elements and determine whether
      // this position should be transmitted to any of the child and more
      // specifically to the `watched` object.
      // When traversing the children we will see whether the first child
      // available for intersection corresponds to the input `watched`
      // object in which case we do not filter the event, otherwise we
      // filter it.
      Guard guard(m_childrenLocker);

      // Traverse the children list and updtae the z order for each one.
      // Note that we actually traverse the `m_chidlren` array in reverse
      // order as the z ordering is sorted so that low order comes first
      // in the vector.
      bool contained = false;
      WidgetsMap::const_reverse_iterator child = m_children.crbegin();
      while (!contained && child != m_children.crend()) {
        contained = child->widget->getRenderingArea().contains(lpos);

        if (!contained) {
          ++child;
        }
      }

      // Check whether we reached a child containing the current mouse
      // position.
      if (!contained) {
        // No child intersected the mouse position which means that the
        // `watched` object certainly doesn't. We can filter the event.
        return true;
      }

      // We found one of the children widget which intersected the mouse
      // position. If this widget corresponds to the input `watched`
      // object it's okay, otherwise it means that another child widget
      // should intercept the element before the `watched` object and
      // thus it should not receive the event.
      return (watched != child->widget);
    }

  }
}
